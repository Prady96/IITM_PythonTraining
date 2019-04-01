
    return this.fieldDetails[index];
  }

  get parsingFinished() {
    return this.parsingIndex >= this._elements.length;
  }

  _pushToSection(name, fieldDetail) {
    for (let section of this._sections) {
      if (section.name == name) {
        section.fieldDetails.push(fieldDetail);
        return;
      }
    }
    this._sections.push({
      name,
      fieldDetails: [fieldDetail],
    });
  }

  _classifySections() {
    let fieldDetails = this._sections[0].fieldDetails;
    this._sections = [];
    let seenTypes = new Set();
    let previousType;
    let sectionCount = 0;

    for (let fieldDetail of fieldDetails) {
      if (!fieldDetail.fieldName) {
        continue;
      }
      if (seenTypes.has(fieldDetail.fieldName) &&
          previousType != fieldDetail.fieldName) {
        seenTypes.clear();
        sectionCount++;
      }
      previousType = fieldDetail.fieldName;
      seenTypes.add(fieldDetail.fieldName);
      this._pushToSection(DEFAULT_SECTION_NAME + "-" + sectionCount, fieldDetail);
    }
  }

  /**
   * The result is an array contains the sections with its belonging field
   * details. If `this._sections` contains one section only with the default
   * section name (DEFAULT_SECTION_NAME), `this._classifySections` should be
   * able to identify all sections in the heuristic way.
   *
   * @returns {Array<Object>}
   *          The array with the sections, and the belonging fieldDetails are in
   *          each section.
   */
  getSectionFieldDetails() {
    // When the section feature is disabled, `getSectionFieldDetails` should
    // provide a single address and credit card section result.
    if (!this._sectionEnabled) {
      return this._getFinalDetails(this.fieldDetails);
    }
    if (this._sections.length == 0) {
      return [];
    }
    if (this._sections.length == 1 && this._sections[0].name == DEFAULT_SECTION_NAME) {
      this._classifySections();
    }

    return this._sections.reduce((sections, current) => {
      sections.push(...this._getFinalDetails(current.fieldDetails));
      return sections;
    }, []);
  }

  /**
   * This function will prepare an autocomplete info object with getInfo
   * function and push the detail to fieldDetails property.
   * Any field will be pushed into `this._sections` based on the section name
   * in `autocomplete` attribute.
   *
   * Any element without the related detail will be used for adding the detail
   * to the end of field details.
   */
  pushDetail() {
    let elementIndex = this.fieldDetails.length;
    if (elementIndex >= this._elements.length) {
      throw new Error("Try to push the non-existing element info.");
    }
    let element = this._elements[elementIndex];
    let info = FormAutofillHeuristics.getInfo(element);
    let fieldInfo = {
      section: info ? info.section : "",
      addressType: info ? info.addressType : "",
      contactType: info ? info.contactType : "",
      fieldName: info ? info.fieldName : "",
      elementWeakRef: Cu.getWeakReference(element),
    };

    if (info && info._reason) {
      fieldInfo._reason = info._reason;
    }

    this.fieldDetails.push(fieldInfo);
    this._pushToSection(this._getSectionName(fieldInfo), fieldInfo);
  }

  _getSectionName(info) {
    let names = [];
    if (info.section) {
      names.push(info.section);
    }
    if (info.addressType) {
      names.push(info.addressType);
    }
    return names.length ? names.join(" ") : DEFAULT_SECTION_NAME;
  }

  /**
   * When a field detail should be changed its fieldName after parsing, use
   * this function to update the fieldName which is at a specific index.
   *
   * @param {number} index
   *        The index indicates a field detail to be updated.
   * @param {string} fieldName
   *        The new fieldName
   */
  updateFieldName(index, fieldName) {
    if (index >= this.fieldDetails.length) {
      throw new Error("Try to update the non-existing field detail.");
    }
    this.fieldDetails[index].fieldName = fieldName;
  }

  _isSameField(field1, field2) {
    return field1.section == field2.section &&
           field1.addressType == field2.addressType &&
           field1.fieldName == field2.fieldName;
  }

  /**
   * Provide the final field details without invalid field name, and the
   * duplicated fields will be removed as well. For the debugging purpose,
   * the final `fieldDetails` will include the duplicated fields if
   * `_allowDuplicates` is true.
   *
   * Each item should contain one type of fields only, and the two valid types
   * are Address and CreditCard.
   *
   * @param   {Array<Object>} fieldDetails
   *          The field details for trimming.
   * @returns {Array<Object>}
   *          The array with the field details without invalid field name and
   *          duplicated fields.
   */
  _getFinalDetails(fieldDetails) {
    let addressFieldDetails = [];
    let creditCardFieldDetails = [];
    for (let fieldDetail of fieldDetails) {
      let fieldName = fieldDetail.fieldName;
      if (FormAutofillUtils.isAddressField(fieldName)) {
        addressFieldDetails.push(fieldDetail);
      } else if (FormAutofillUtils.isCreditCardField(fieldName)) {
        creditCardFieldDetails.push(fieldDetail);
      } else {
        log.debug("Not collecting a field with a unknown fieldName", fieldDetail);
      }
    }

    return [
      {
        type: FormAutofillUtils.SECTION_TYPES.ADDRESS,
        fieldDetails: addressFieldDetails,
      },
      {
        type: FormAutofillUtils.SECTION_TYPES.CREDIT_CARD,
        fieldDetails: creditCardFieldDetails,
      },
    ].map(section => {
      if (this._allowDuplicates) {
        return section;
      }
      // Deduplicate each set of fieldDetails
      let details = section.fieldDetails;
      section.fieldDetails = details.filter((detail, index) => {
        let previousFields = details.slice(0, index);
        return !previousFields.find(f => this._isSameField(detail, f));
      });
      return section;
    }).filter(section => section.fieldDetails.length > 0);
  }

  elementExisting(index) {
    return index < this._elements.length;
  }
}

var LabelUtils = {
  // The tag name list is from Chromium except for "STYLE":
  // eslint-disable-next-line max-len
  // https://cs.chromium.org/chromium/src/components/autofill/content/renderer/form_autofill_util.cc?l=216&rcl=d33a171b7c308a64dc3372fac3da2179c63b419e
  EXCLUDED_TAGS: ["SCRIPT", "NOSCRIPT", "OPTION", "STYLE"],

  // A map object, whose keys are the id's of form fields and each value is an
  // array consisting of label elements correponding to the id.
  // @type {Map<string, array>}
  _mappedLabels: null,

  // An array consisting of label elements whose correponding form field doesn't
  // have an id attribute.
  // @type {Array<HTMLLabelElement>}
  _unmappedLabels: null,

  // A weak map consisting of label element and extracted strings pairs.
  // @type {WeakMap<HTMLLabelElement, array>}
  _labelStrings: null,

  /**
   * Extract all strings of an element's children to an array.
   * "element.textContent" is a string which is merged of all children nodes,
   * and this function provides an array of the strings contains in an element.
   *
   * @param  {Object} element
   *         A DOM element to be extracted.
   * @returns {Array}
   *          All strings in an element.
   */
  extractLabelStrings(element) {
    if (this._labelStrings.has(element)) {
      return this._labelStrings.get(element);
    }
    let strings = [];
    let _extractLabelStrings = (el) => {
      if (this.EXCLUDED_TAGS.includes(el.tagName)) {
        return;
      }

      if (el.nodeType == el.TEXT_NODE || el.childNodes.length == 0) {
        let trimmedText = el.textContent.trim();
        if (trimmedText) {
          strings.push(trimmedText);
        }
        return;
      }

      for (let node of el.childNodes) {
        let nodeType = node.nodeType;
        if (nodeType != node.ELEMENT_NODE && nodeType != node.TEXT_NODE) {
          continue;
        }
        _extractLabelStrings(node);
      }
    };
    _extractLabelStrings(element);
    this._labelStrings.set(element, strings);
    return strings;
  },

  generateLabelMap(doc) {
    let mappedLabels = new Map();
    let unmappedLabels = [];

    for (let label of doc.querySelectorAll("label")) {
      let id = label.htmlFor;
      if (!id) {
        let control = label.control;
        if (!control) {
          continue;
        }
        id = control.id;
      }
      if (id) {
        let labels = mappedLabels.get(id);
        if (labels) {
          labels.push(label);
        } else {
          mappedLabels.set(id, [label]);
        }
      } else {
        unmappedLabels.push(label);
      }
    }

    this._mappedLabels = mappedLabels;
    this._unmappedLabels = unmappedLabels;
    this._labelStrings = new WeakMap();
  },

  clearLabelMap() {
    this._mappedLabels = null;
    this._unmappedLabels = null;
    this._labelStrings = null;
  },

  findLabelElements(element) {
    if (!this._mappedLabels) {
      this.generateLabelMap(element.ownerDocument);
    }

    let id = element.id;
    if (!id) {
      return this._unmappedLabels.filter(label => label.control == element);
    }
    return this._mappedLabels.get(id) || [];
  },
};

/**
 * Returns the autocomplete information of fields according to heuristics.
 */
this.FormAutofillHeuristics = {
  RULES: null,

  /**
   * Try to find a contiguous sub-array within an array.
   *
   * @param {Array} array
   * @param {Array} subArray
   *
   * @returns {boolean}
   *          Return whether subArray was found within the array or not.
   */
  _matchContiguousSubArray(array, subArray) {
    return array.some((elm, i) => subArray.every((sElem, j) => sElem == array[i + j]));
  },

  /**
   * Try to find the field that is look like a month select.
   *
   * @param {DOMElement} element
   * @returns {boolean}
   *          Return true if we observe the trait of month select in
   *          the current element.
   */
  _isExpirationMonthLikely(element) {
    if (ChromeUtils.getClassName(element) !== "HTMLSelectElement") {
      return false;
    }

    const options = [...element.options];
    const desiredValues = Array(12).fill(1).map((v, i) => v + i);

    // The number of month options shouldn't be less than 12 or larger than 13
    // including the default option.
    if (options.length < 12 || options.length > 13) {
      return false;
    }

    return this._matchContiguousSubArray(options.map(e => +e.value), desiredValues) ||
           this._matchContiguousSubArray(options.map(e => +e.label), desiredValues);
  },


  /**
   * Try to find the field that is look like a year select.
   *
   * @param {DOMElement} element
   * @returns {boolean}
   *          Return true if we observe the trait of year select in
   *          the current element.
   */
  _isExpirationYearLikely(element) {
    if (ChromeUtils.getClassName(element) !== "HTMLSelectElement") {
      return false;
    }

    const options = [...element.options];
    // A normal expiration year select should contain at least the last three years
    // in the list.
    const curYear = new Date().getFullYear();
    const desiredValues = Array(3).fill(0).map((v, i) => v + curYear + i);

    return this._matchContiguousSubArray(options.map(e => +e.value), desiredValues) ||
           this._matchContiguousSubArray(options.map(e => +e.label), desiredValues);
  },


  /**
   * Try to match the telephone related fields to the grammar
   * list to see if there is any valid telephone set and correct their
   * field names.
   *
   * @param {FieldScanner} fieldScanner
   *        The current parsing status for all elements
   * @returns {boolean}
   *          Return true if there is any field can be recognized in the parser,
   *          otherwise false.
   */
  _parsePhoneFields(fieldScanner) {
    let matchingResult;

    const GRAMMARS = this.PHONE_FIELD_GRAMMARS;
    for (let i = 0; i < GRAMMARS.length; i++) {
      let detailStart = fieldScanner.parsingIndex;
      let ruleStart = i;
      for (; i < GRAMMARS.length && GRAMMARS[i][0] && fieldScanner.elementExisting(detailStart); i++, detailStart++) {
        let detail = fieldScanner.getFieldDetailByIndex(detailStart);
        if (!detail || GRAMMARS[i][0] != detail.fieldName || (detail._reason && detail._reason == "autocomplete")) {
          break;
        }
        let element = detail.elementWeakRef.get();
        if (!element) {
          break;
        }
        if (GRAMMARS[i][2] && (!element.maxLength || GRAMMARS[i][2] < element.maxLength)) {
          break;
        }
      }
      if (i >= GRAMMARS.length) {
        break;
      }

      if (!GRAMMARS[i][0]) {
        matchingResult = {
          ruleFrom: ruleStart,
          ruleTo: i,
        };
        break;
      }

      // Fast rewinding to the next rule.
      for (; i < GRAMMARS.length; i++) {
        if (!GRAMMARS[i][0]) {
          break;
        }
      }
    }

    let parsedField = false;
    if (matchingResult) {
      let {ruleFrom, ruleTo} = matchingResult;
      let detailStart = fieldScanner.parsingIndex;
      for (let i = ruleFrom; i < ruleTo; i++) {
        fieldScanner.updateFieldName(detailStart, GRAMMARS[i][1]);
        fieldScanner.parsingIndex++;
        detailStart++;
        parsedField = true;
      }
    }

    if (fieldScanner.parsingFinished) {
      return parsedField;
    }

    let nextField = fieldScanner.getFieldDetailByIndex(fieldScanner.parsingIndex);
    if (nextField && nextField._reason != "autocomplete" && fieldScanner.parsingIndex > 0) {
      const regExpTelExtension = new RegExp(
        "\\bext|ext\\b|extension" +
        "|ramal", // pt-BR, pt-PT
        "iu");
      const previousField = fieldScanner.getFieldDetailByIndex(fieldScanner.parsingIndex - 1);
      const previousFieldType = FormAutofillUtils.getCategoryFromFieldName(previousField.fieldName);
      if (previousField && previousFieldType == "tel" &&
        this._matchRegexp(nextField.elementWeakRef.get(), regExpTelExtension)) {
        fieldScanner.updateFieldName(fieldScanner.parsingIndex, "tel-extension");
        fieldScanner.parsingIndex++;
        parsedField = true;
      }
    }

    return parsedField;
  },

  /**
   * Try to find the correct address-line[1-3] sequence and correct their field
   * names.
   *
   * @param {FieldScanner} fieldScanner
   *        The current parsing status for all elements
   * @returns {boolean}
   *          Return true if there is any field can be recognized in the parser,
   *          otherwise false.
   */
  _parseAddressFields(fieldScanner) {
    let parsedFields = false;
    const addressLines = ["address-line1", "address-line2", "address-line3"];

    // TODO: These address-line* regexps are for the lines with numbers, and
    // they are the subset of the regexps in `heuristicsRegexp.js`. We have to
    // find a better way to make them consistent.
    const addressLineRegexps = {
      "address-line1": new RegExp(
        "address[_-]?line(1|one)|address1|addr1" +
        "|addrline1|address_1" + // Extra rules by Firefox
        "|indirizzo1" + // it-IT
        "|