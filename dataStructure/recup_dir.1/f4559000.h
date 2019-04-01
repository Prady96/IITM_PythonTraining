r (let regexp of Object.keys(addressLineRegexps)) {
        if (this._matchRegexp(elem, addressLineRegexps[regexp])) {
          fieldScanner.updateFieldName(fieldScanner.parsingIndex, regexp);
          parsedFields = true;
        }
      }
      fieldScanner.parsingIndex++;
    }

    return parsedFields;
  },

  /**
   * Try to look for expiration date fields and revise the field names if needed.
   *
   * @param {FieldScanner} fieldScanner
   *        The current parsing status for all elements
   * @returns {boolean}
   *          Return true if there is any field can be recognized in the parser,
   *          otherwise false.
   */
  _parseCreditCardExpirationDateFields(fieldScanner) {
    if (fieldScanner.parsingFinished) {
      return false;
    }

    const savedIndex = fieldScanner.parsingIndex;
    const monthAndYearFieldNames = ["cc-exp-month", "cc-exp-year"];
    const detail = fieldScanner.getFieldDetailByIndex(fieldScanner.parsingIndex);
    const element = detail.elementWeakRef.get();

    // Respect to autocomplete attr and skip the uninteresting fields
    if (!detail || (detail._reason && detail._reason == "autocomplete") ||
        !["cc-exp", ...monthAndYearFieldNames].includes(detail.fieldName)) {
      return false;
    }

    // If the input type is a month picker, then assume it's cc-exp.
    if (element.type == "month") {
      fieldScanner.updateFieldName(fieldScanner.parsingIndex, "cc-exp");
      fieldScanner.parsingIndex++;

      return true;
    }

    // Don't process the fields if expiration month and expiration year are already
    // matched by regex in correct order.
    if (fieldScanner.getFieldDetailByIndex(fieldScanner.parsingIndex++).fieldName == "cc-exp-month" &&
        !fieldScanner.parsingFinished &&
        fieldScanner.getFieldDetailByIndex(fieldScanner.parsingIndex++).fieldName == "cc-exp-year") {
      return true;
    }
    fieldScanner.parsingIndex = savedIndex;

    // Determine the field name by checking if the fields are month select and year select
    // likely.
    if (this._isExpirationMonthLikely(element)) {
      fieldScanner.updateFieldName(fieldScanner.parsingIndex, "cc-exp-month");
      fieldScanner.parsingIndex++;
      if (!fieldScanner.parsingFinished) {
        const nextDetail = fieldScanner.getFieldDetailByIndex(fieldScanner.parsingIndex);
        const nextElement = nextDetail.elementWeakRef.get();
        if (this._isExpirationYearLikely(nextElement)) {
          fieldScanner.updateFieldName(fieldScanner.parsingIndex, "cc-exp-year");
          fieldScanner.parsingIndex++;
          return true;
        }
      }
    }
    fieldScanner.parsingIndex = savedIndex;

    // Verify that the following consecutive two fields can match cc-exp-month and cc-exp-year
    // respectively.
    if (this._findMatchedFieldName(element, ["cc-exp-month"])) {
      fieldScanner.updateFieldName(fieldScanner.parsingIndex, "cc-exp-month");
      fieldScanner.parsingIndex++;
      if (!fieldScanner.parsingFinished) {
        const nextDetail = fieldScanner.getFieldDetailByIndex(fieldScanner.parsingIndex);
        const nextElement = nextDetail.elementWeakRef.get();
        if (this._findMatchedFieldName(nextElement, ["cc-exp-year"])) {
          fieldScanner.updateFieldName(fieldScanner.parsingIndex, "cc-exp-year");
          fieldScanner.parsingIndex++;
          return true;
        }
      }
    }
    fieldScanner.parsingIndex = savedIndex;

    // Look for MM and/or YY(YY).
    if (this._matchRegexp(element, /^mm$/ig)) {
      fieldScanner.updateFieldName(fieldScanner.parsingIndex, "cc-exp-month");
      fieldScanner.parsingIndex++;
      if (!fieldScanner.parsingFinished) {
        const nextDetail = fieldScanner.getFieldDetailByIndex(fieldScanner.parsingIndex);
        const nextElement = nextDetail.elementWeakRef.get();
        if (this._matchRegexp(nextElement, /^(yy|yyyy)$/)) {
          fieldScanner.updateFieldName(fieldScanner.parsingIndex, "cc-exp-year");
          fieldScanner.parsingIndex++;

          return true;
        }
      }
    }
    fieldScanner.parsingIndex = savedIndex;

    // Look for a cc-exp with 2-digit or 4-digit year.
    if (this._matchRegexp(element, /(?:exp.*date[^y\\n\\r]*|mm\\s*[-/]?\\s*)yy(?:[^y]|$)/ig) ||
        this._matchRegexp(element, /(?:exp.*date[^y\\n\\r]*|mm\\s*[-/]?\\s*)yyyy(?:[^y]|$)/ig)) {
      fieldScanner.updateFieldName(fieldScanner.parsingIndex, "cc-exp");
      fieldScanner.parsingIndex++;
      return true;
    }
    fieldScanner.parsingIndex = savedIndex;

    // Match general cc-exp regexp at last.
    if (this._findMatchedFieldName(element, ["cc-exp"])) {
      fieldScanner.updateFieldName(fieldScanner.parsingIndex, "cc-exp");
      fieldScanner.parsingIndex++;
      return true;
    }
    fieldScanner.parsingIndex = savedIndex;

    // Set current field name to null as it failed to match any patterns.
    fieldScanner.updateFieldName(fieldScanner.parsingIndex, null);
    fieldScanner.parsingIndex++;
    return true;
  },

  /**
   * This function should provide all field details of a form which are placed
   * in the belonging section. The details contain the autocomplete info
   * (e.g. fieldName, section, etc).
   *
   * `allowDuplicates` is used for the xpcshell-test purpose currently because
   * the heuristics should be verified that some duplicated elements still can
   * be predicted correctly.
   *
   * @param {HTMLFormElement} form
   *        the elements in this form to be predicted the field info.
   * @param {boolean} allowDuplicates
   *        true to remain any duplicated field details otherwise to remove the
   *        duplicated ones.
   * @returns {Array<Array<Object>>}
   *        all sections within its field details in the form.
   */
  getFormInfo(form, allowDuplicates = false) {
    const eligibleFields = Array.from(form.elements)
      .filter(elem => FormAutofillUtils.isFieldEligibleForAutofill(elem));

    if (eligibleFields.length <= 0) {
      return [];
    }

    let fieldScanner = new FieldScanner(eligibleFields,
      {allowDuplicates, sectionEnabled: this._sectionEnabled});
    while (!fieldScanner.parsingFinished) {
      let parsedPhoneFields = this._parsePhoneFields(fieldScanner);
      let parsedAddressFields = this._parseAddressFields(fieldScanner);
      let parsedExpirationDateFields = this._parseCreditCardExpirationDateFields(fieldScanner);

      // If there is no any field parsed, the parsing cursor can be moved
      // forward to the next one.
      if (!parsedPhoneFields && !parsedAddressFields && !parsedExpirationDateFields) {
        fieldScanner.parsingIndex++;
      }
    }

    LabelUtils.clearLabelMap();

    return fieldScanner.getSectionFieldDetails();
  },

  _regExpTableHashValue(...signBits) {
    return signBits.reduce((p, c, i) => p | !!c << i, 0);
  },

  _setRegExpListCache(regexps, b0, b1, b2) {
    if (!this._regexpList) {
      this._regexpList = [];
    }
    this._regexpList[this._regExpTableHashValue(b0, b1, b2)] = regexps;
  },

  _getRegExpListCache(b0, b1, b2) {
    if (!this._regexpList) {
      return null;
    }
    return this._regexpList[this._regExpTableHashValue(b0, b1, b2)] || null;
  },

  _getRegExpList(isAutoCompleteOff, elementTagName) {
    let isSelectElem = elementTagName == "SELECT";
    let regExpListCache = this._getRegExpListCache(
      isAutoCompleteOff,
      FormAutofill.isAutofillCreditCardsAvailable,
      isSelectElem
    );
    if (regExpListCache) {
      return regExpListCache;
    }
    const FIELDNAMES_IGNORING_AUTOCOMPLETE_OFF = [
      "cc-name",
      "cc-number",
      "cc-exp-month",
      "cc-exp-year",
      "cc-exp",
    ];
    let regexps = isAutoCompleteOff ? FIELDNAMES_IGNORING_AUTOCOMPLETE_OFF : Object.keys(this.RULES);

    if (!FormAutofill.isAutofillCreditCardsAvailable) {
      regexps = regexps.filter(name => !FormAutofillUtils.isCreditCardField(name));
    }

    if (isSelectElem) {
      const FIELDNAMES_FOR_SELECT_ELEMENT = [
        "address-level1",
        "address-level2",
        "country",
        "cc-exp-month",
        "cc-exp-year",
        "cc-exp",
      ];
      regexps = regexps.filter(name => FIELDNAMES_FOR_SELECT_ELEMENT.includes(name));
    }

    this._setRegExpListCache(
      regexps,
      isAutoCompleteOff,
      FormAutofill.isAutofillCreditCardsAvailable,
      isSelectElem
    );

    return regexps;
  },

  getInfo(element) {
    let info = element.getAutocompleteInfo();
    // An input[autocomplete="on"] will not be early return here since it stll
    // needs to find the field name.
    if (info && info.fieldName && info.fieldName != "on" && info.fieldName != "off") {
      info._reason = "autocomplete";
      return info;
    }

    if (!this._prefEnabled) {
      return null;
    }

    let isAutoCompleteOff = element.autocomplete == "off" ||
      (element.form && element.form.autocomplete == "off");

    // "email" type of input is accurate for heuristics to determine its Email
    // field or not. However, "tel" type is used for ZIP code for some web site
    // (e.g. HomeDepot, BestBuy), so "tel" type should be not used for "tel"
    // prediction.
    if (element.type == "email" && !isAutoCompleteOff) {
      return {
        fieldName: "email",
        section: "",
        addressType: "",
        contactType: "",
      };
    }

    let regexps = this._getRegExpList(isAutoCompleteOff, element.tagName);
    if (regexps.length == 0) {
      return null;
    }

    let matchedFieldName =  this._findMatchedFieldName(element, regexps);
    if (matchedFieldName) {
      return {
        fieldName: matchedFieldName,
        section: "",
        addressType: "",
        contactType: "",
      };
    }

    return null;
  },

  /**
   * @typedef ElementStrings
   * @type {object}
   * @yield {string} id - element id.
   * @yield {string} name - element name.
   * @yield {Array<string>} labels - extracted labels.
   */

  /**
   * Extract all the signature strings of an element.
   *
   * @param {HTMLElement} element
   * @returns {ElementStrings}
   */
  _getElementStrings(element) {
    return {
      * [Symbol.iterator]() {
        yield element.id;
        yield element.name;

        const labels = LabelUtils.findLabelElements(element);
        for (let label of labels) {
          yield *LabelUtils.extractLabelStrings(label);
        }
      },
    };
  },

  /**
   * Find the first matched field name of the element wih given regex list.
   *
   * @param {HTMLElement} element
   * @param {Array<string>} regexps
   *        The regex key names that correspond to pattern in the rule list.
   * @returns {?string} The first matched field name
   */
  _findMatchedFieldName(element, regexps) {
    const getElementStrings = this._getElementStrings(element);
    for (let regexp of regexps) {
      for (let string of getElementStrings) {
        // The original regexp "(?<!united )state|county|region|province" for
        // "address-line1" wants to exclude any "united state" string, so the
        // following code is to remove all "united state" string before applying
        // "addess-level1" regexp.
        //
        // Since "united state" string matches to the regexp of address-line2&3,
        // the two regexps should be excluded here.
        if (["address-level1", "address-line2", "address-line3"].includes(regexp)) {
          string = string.toLowerCase().split("united state").join("");
        }
        if (this.RULES[regexp].test(string)) {
          return regexp;
        }
      }
    }

    return null;
  },

  /**
   * Determine whether the regexp can match any of element strings.
   *
   * @param {HTMLElement} element
   * @param {RegExp} regexp
   *
   * @returns {boolean}
   */
  _matchRegexp(element, regexp) {
    const elemStrings = this._getElementStrings(element);
    for (const str of elemStrings) {
      if (regexp.test(str)) {
        return true;
      }
    }
    return false;
  },

/**
 * Phone field grammars - first matched grammar will be parsed. Grammars are
 * separated by { REGEX_SEPARATOR, FIELD_NONE, 0 }. Suffix and extension are
 * parsed separately unless they are necessary parts of the match.
 * The following notation is used to describe the patterns:
 * <cc> - country code field.
 * <ac> - area code field.
 * <phone> - phone or prefix.
 * <suffix> - suffix.
 * <ext> - extension.
 * :N means field is limited to N characters, otherwise it is unlimited.
 * (pattern <field>)? means pattern is optional and matched separately.
 *
 * This grammar list from Chromium will be enabled partially once we need to
 * support more cases of Telephone fields.
 */
  PHONE_FIELD_GRAMMARS: [
    // Country code: <cc> Area Code: <ac> Phone: <phone> (- <suffix>

    // (Ext: <ext>)?)?
      // {REGEX_COUNTRY, FIELD_COUNTRY_CODE, 0},
      // {REGEX_AREA, FIELD_AREA_CODE, 0},
      // {REGEX_PHONE, FIELD_PHONE, 0},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // \( <ac> \) <phone>:3 <suffix>:4 (Ext: <ext>)?
      // {REGEX_AREA_NOTEXT, FIELD_AREA_CODE, 3},
      // {REGEX_PREFIX_SEPARATOR, FIELD_PHONE, 3},
      // {REGEX_PHONE, FIELD_SUFFIX, 4},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Phone: <cc> <ac>:3 - <phone>:3 - <suffix>:4 (Ext: <ext>)?
      // {REGEX_PHONE, FIELD_COUNTRY_CODE, 0},
      // {REGEX_PHONE, FIELD_AREA_CODE, 3},
      // {REGEX_PREFIX_SEPARATOR, FIELD_PHONE, 3},
      // {REGEX_SUFFIX_SEPARATOR, FIELD_SUFFIX, 4},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Phone: <cc>:3 <ac>:3 <phone>:3 <suffix>:4 (Ext: <ext>)?
    ["tel", "tel-country-code", 3],
    ["tel", "tel-area-code", 3],
    ["tel", "tel-local-prefix", 3],
    ["tel", "tel-local-suffix", 4],
    [null, null, 0],

    // Area Code: <ac> Phone: <phone> (- <suffix> (Ext: <ext>)?)?
      // {REGEX_AREA, FIELD_AREA_CODE, 0},
      // {REGEX_PHONE, FIELD_PHONE, 0},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Phone: <ac> <phone>:3 <suffix>:4 (Ext: <ext>)?
      // {REGEX_PHONE, FIELD_AREA_CODE, 0},
      // {REGEX_PHONE, FIELD_PHONE, 3},
      // {REGEX_PHONE, FIELD_SUFFIX, 4},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Phone: <cc> \( <ac> \) <phone> (- <suffix> (Ext: <ext>)?)?
      // {REGEX_PHONE, FIELD_COUNTRY_CODE, 0},
      // {REGEX_AREA_NOTEXT, FIELD_AREA_CODE, 0},
      // {REGEX_PREFIX_SEPARATOR, FIELD_PHONE, 0},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Phone: \( <ac> \) <phone> (- <suffix> (Ext: <ext>)?)?
      // {REGEX_PHONE, FIELD_COUNTRY_CODE, 0},
      // {REGEX_AREA_NOTEXT, FIELD_AREA_CODE, 0},
      // {REGEX_PREFIX_SEPARATOR, FIELD_PHONE, 0},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Phone: <cc> - <ac> - <phone> - <suffix> (Ext: <ext>)?
      // {REGEX_PHONE, FIELD_COUNTRY_CODE, 0},
      // {REGEX_PREFIX_SEPARATOR, FIELD_AREA_CODE, 0},
      // {REGEX_PREFIX_SEPARATOR, FIELD_PHONE, 0},
      // {REGEX_SUFFIX_SEPARATOR, FIELD_SUFFIX, 0},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Area code: <ac>:3 Prefix: <prefix>:3 Suffix: <suffix>:4 (Ext: <ext>)?
      // {REGEX_AREA, FIELD_AREA_CODE, 3},
      // {REGEX_PREFIX, FIELD_PHONE, 3},
      // {REGEX_SUFFIX, FIELD_SUFFIX, 4},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Phone: <ac> Prefix: <phone> Suffix: <suffix> (Ext: <ext>)?
      // {REGEX_PHONE, FIELD_AREA_CODE, 0},
      // {REGEX_PREFIX, FIELD_PHONE, 0},
      // {REGEX_SUFFIX, FIELD_SUFFIX, 0},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Phone: <ac> - <phone>:3 - <suffix>:4 (Ext: <ext>)?
    ["tel", "tel-area-code", 0],
    ["tel", "tel-local-prefix", 3],
    ["tel", "tel-local-suffix", 4],
    [null, null, 0],

    // Phone: <cc> - <ac> - <phone> (Ext: <ext>)?
      // {REGEX_PHONE, FIELD_COUNTRY_CODE, 0},
      // {REGEX_PREFIX_SEPARATOR, FIELD_AREA_CODE, 0},
      // {REGEX_SUFFIX_SEPARATOR, FIELD_PHONE, 0},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Phone: <ac> - <phone> (Ext: <ext>)?
      // {REGEX_AREA, FIELD_AREA_CODE, 0},
      // {REGEX_PHONE, FIELD_PHONE, 0},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Phone: <cc>:3 - <phone>:10 (Ext: <ext>)?
      // {REGEX_PHONE, FIELD_COUNTRY_CODE, 3},
      // {REGEX_PHONE, FIELD_PHONE, 10},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Ext: <ext>
      // {REGEX_EXTENSION, FIELD_EXTENSION, 0},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},

    // Phone: <phone> (Ext: <ext>)?
      // {REGEX_PHONE, FIELD_PHONE, 0},
      // {REGEX_SEPARATOR, FIELD_NONE, 0},
  ],
};

XPCOMUtils.defineLazyGetter(this.FormAutofillHeuristics, "RULES", () => {
  let sandbox = {};
  const HEURISTICS_REGEXP = "chrome://formautofill/content/heuristicsRegexp.js";
  Services.scriptloader.loadSubScript(HEURISTICS_REGEXP, sandbox);
  return sandbox.HeuristicsRegExp.RULES;
});

XPCOMUtils.defineLazyGetter(this.FormAutofillHeuristics, "_prefEnabled", () => {
  return Services.prefs.getBoolPref(PREF_HEURISTICS_ENABLED);
});

Services.prefs.addObserver(PREF_HEURISTICS_ENABLED, () => {
  this.FormAutofillHeuristics._prefEnabled = Services.prefs.getBoolPref(PREF_HEURISTICS_ENABLED);
});

XPCOMUtils.defineLazyGetter(this.FormAutofillHeuristics, "_sectionEnabled", () => {
  return Services.prefs.getBoolPref(PREF_SECTION_ENABLED);
});

Services.prefs.addObserver(PREF_SECTION_ENABLED, () => {
  this.FormAutofillHeuristics._sectionEnabled = Services.prefs.getBoolPref(PREF_SECTION_ENABLED);
});
PK