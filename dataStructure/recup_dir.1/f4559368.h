ldNames, profiles) {}

  /**
   * Get the value of the result at the given index.
   *
   * Always return empty string for form autofill feature to suppress
   * AutoCompleteController from autofilling, as we'll populate the
   * fields on our own.
   *
   * @param   {number} index The index of the result requested
   * @returns {string} The result at the specified index
   */
  getValueAt(index) {
    this._checkIndexBounds(index);
    return "";
  }

  getLabelAt(index) {
    this._checkIndexBounds(index);

    let label = this._popupLabels[index];
    if (typeof label == "string") {
      return label;
    }
    return JSON.stringify(label);
  }

  /**
   * Retrieves a comment (metadata instance)
   * @param   {number} index The index of the comment requested
   * @returns {string} The comment at the specified index
   */
  getCommentAt(index) {
    this._checkIndexBounds(index);
    return JSON.stringify(this._matchingProfiles[index]);
  }

  /**
   * Retrieves a style hint specific to a particular index.
   * @param   {number} index The index of the style hint requested
   * @returns {string} The style hint at the specified index
   */
  getStyleAt(index) {
    this._checkIndexBounds(index);
    if (index == this.matchCount - 1) {
      return "autofill-footer";
    }
    if (this._isInputAutofilled) {
      return "autofill-clear-button";
    }

    return "autofill-profile";
  }

  /**
   * Retrieves an image url.
   * @param   {number} index The index of the image url requested
   * @returns {string} The image url at the specified index
   */
  getImageAt(index) {
    this._checkIndexBounds(index);
    return "";
  }

  /**
   * Retrieves a result
   * @param   {number} index The index of the result requested
   * @returns {string} The result at the specified index
   */
  getFinalCompleteValueAt(index) {
    return this.getValueAt(index);
  }

  /**
   * Removes a result from the resultset
   * @param {number} index The index of the result to remove
   * @param {boolean} removeFromDatabase TRUE for removing data from DataBase
   *                                     as well.
   */
  removeValueAt(index, removeFromDatabase) {
    // There is no plan to support removing profiles via autocomplete.
  }
}

class AddressResult extends ProfileAutoCompleteResult {
  constructor(...args) {
    super(...args);
  }

  _getSecondaryLabel(focusedFieldName, allFieldNames, profile) {
    // We group similar fields into the same field name so we won't pick another
    // field in the same group as the secondary label.
    const GROUP_FIELDS = {
      "name": [
        "name",
        "given-name",
        "additional-name",
        "family-name",
      ],
      "street-address": [
        "street-address",
        "address-line1",
        "address-line2",
        "address-line3",
      ],
      "country-name": [
        "country",
        "country-name",
      ],
      "tel": [
        "tel",
        "tel-country-code",
        "tel-national",
        "tel-area-code",
        "tel-local",
        "tel-local-prefix",
        "tel-local-suffix",
      ],
    };

    const secondaryLabelOrder = [
      "street-address",  // Street address
      "name",            // Full name
      "address-level3",  // Townland / Neighborhood / Village
      "address-level2",  // City/Town
      "organization",    // Company or organization name
      "address-level1",  // Province/State (Standardized code if possible)
      "country-name",    // Country name
      "postal-code",     // Postal code
      "tel",             // Phone number
      "email",           // Email address
    ];

    for (let field in GROUP_FIELDS) {
      if (GROUP_FIELDS[field].includes(focusedFieldName)) {
        focusedFieldName = field;
        break;
      }
    }

    for (const currentFieldName of secondaryLabelOrder) {
      if (focusedFieldName == currentFieldName || !profile[currentFieldName]) {
        continue;
      }

      let matching = GROUP_FIELDS[currentFieldName] ?
        allFieldNames.some(fieldName => GROUP_FIELDS[currentFieldName].includes(fieldName)) :
        allFieldNames.includes(currentFieldName);

      if (matching) {
        if (currentFieldName == "street-address" &&
            profile["-moz-street-address-one-line"]) {
          return profile["-moz-street-address-one-line"];
        }
        return profile[currentFieldName];
      }
    }

    return ""; // Nothing matched.
  }

  _generateLabels(focusedFieldName, allFieldNames, profiles) {
    if (this._isInputAutofilled) {
      return [
        {primary: "", secondary: ""}, // Clear button
        {primary: "", secondary: ""}, // Footer
      ];
    }

    // Skip results without a primary label.
    let labels = profiles.filter(profile => {
      return !!profile[focusedFieldName];
    }).map(profile => {
      let primaryLabel = profile[focusedFieldName];
      if (focusedFieldName == "street-address" &&
          profile["-moz-street-address-one-line"]) {
        primaryLabel = profile["-moz-street-address-one-line"];
      }
      return {
        primary: primaryLabel,
        secondary: this._getSecondaryLabel(focusedFieldName,
                                           allFieldNames,
                                           profile),
      };
    });
    // Add an empty result entry for footer. Its content will come from
    // the footer binding, so don't assign any value to it.
    // The additional properties: categories and focusedCategory are required of
    // the popup to generate autofill hint on the footer.
    labels.push({
      primary: "",
      secondary: "",
      categories: FormAutofillUtils.getCategoriesFromFieldNames(this._allFieldNames),
      focusedCategory: FormAutofillUtils.getCategoryFromFieldName(this._focusedFieldName),
    });

    return labels;
  }
}

class CreditCardResult extends ProfileAutoCompleteResult {
  constructor(...args) {
    super(...args);
  }

  _getSecondaryLabel(focusedFieldName, allFieldNames, profile) {
    const GROUP_FIELDS = {
      "cc-name": [
        "cc-name",
        "cc-given-name",
        "cc-additional-name",
        "cc-family-name",
      ],
      "cc-exp": [
        "cc-exp",
        "cc-exp-month",
        "cc-exp-year",
      ],
    };

    const secondaryLabelOrder = [
      "cc-number",       // Credit card number
      "cc-name",         // Full name
      "cc-exp",          // Expiration date
    ];

    for (let field in GROUP_FIELDS) {
      if (GROUP_FIELDS[field].includes(focusedFieldName)) {
        focusedFieldName = field;
        break;
      }
    }

    for (const currentFieldName of secondaryLabelOrder) {
      if (focusedFieldName == currentFieldName || !profile[currentFieldName]) {
        continue;
      }

      let matching = GROUP_FIELDS[currentFieldName] ?
        allFieldNames.some(fieldName => GROUP_FIELDS[currentFieldName].includes(fieldName)) :
        allFieldNames.includes(currentFieldName);

      if (matching) {
        if (currentFieldName == "cc-number") {
          let {affix, label} = CreditCard.formatMaskedNumber(profile[currentFieldName]);
          return affix + label;
        }
        return profile[currentFieldName];
      }
    }

    return ""; // Nothing matched.
  }

  _generateLabels(focusedFieldName, allFieldNames, profiles) {
    if (!this._isSecure) {
      if (!insecureWarningEnabled) {
        return [];
      }
      let brandName = FormAutofillUtils.brandBundle.GetStringFromName("brandShortName");

      return [FormAutofillUtils.stringBundle.formatStringFromName("insecureFieldWarningDescription", [brandName], 1)];
    }

    if (this._isInputAutofilled) {
      return [
        {primary: "", secondary: ""}, // Clear button
        {primary: "", secondary: ""}, // Footer
      ];
    }

    // Skip results without a primary label.
    let labels = profiles.filter(profile => {
      return !!profile[focusedFieldName];
    }).map(profile => {
      let primaryAffix;
      let primary = profile[focusedFieldName];

      if (focusedFieldName == "cc-number") {
        let {affix, label} = CreditCard.formatMaskedNumber(primary);
        primaryAffix = affix;
        primary = label;
      }
      return {
        primaryAffix,
        primary,
        secondary: this._getSecondaryLabel(focusedFieldName,
                                           allFieldNames,
                                           profile),
      };
    });
    // Add an empty result entry for footer.
    labels.push({primary: "", secondary: ""});

    return labels;
  }

  getStyleAt(index) {
    this._checkIndexBounds(index);
    if (!this._isSecure && insecureWarningEnabled) {
      return "autofill-insecureWarning";
    }

    return super.getStyleAt(index);
  }

  getImageAt(index) {
    this._checkIndexBounds(index);
    return "chrome://formautofill/content/icon-credit-card-generic.svg";
  }
}
PK