 = {
  /**
   * Create the Form Autofill preference group.
   *
   * @param   {XULDocument} document
   * @returns {XULElement}
   */
  init(document) {
    this.createPreferenceGroup(document);
    this.attachEventListeners();

    return this.refs.formAutofillFragment;
  },

  /**
   * Remove event listeners and the preference group.
   */
  uninit() {
    this.detachEventListeners();
    this.refs.formAutofillGroup.remove();
  },

  /**
   * Create Form Autofill preference group
   *
   * @param  {XULDocument} document
   */
  createPreferenceGroup(document) {
    let learnMoreURL = Services.urlFormatter.formatURLPref("app.support.baseURL") + "autofill-card-address";
    let formAutofillFragment = document.createDocumentFragment();
    let formAutofillGroupBoxLabel = document.createXULElement("label");
    let formAutofillGroupBoxLabelHeading = document.createElementNS(HTML_NS, "h2");
    let formAutofillGroup = document.createXULElement("vbox");
    let addressAutofill = document.createXULElement("hbox");
    let addressAutofillCheckboxGroup = document.createXULElement("hbox");
    let addressAutofillCheckbox = document.createXULElement("checkbox");
    let addressAutofillLearnMore = document.createXULElement("label");
    let savedAddressesBtn = document.createXULElement("button");
    // Wrappers are used to properly compute the search tooltip positions
    let savedAddressesBtnWrapper = document.createXULElement("hbox");
    let savedCreditCardsBtnWrapper = document.createXULElement("hbox");

    savedAddressesBtn.className = "accessory-button";
    addressAutofillCheckbox.className = "tail-with-learn-more";
    addressAutofillLearnMore.className = "learnMore text-link";

    formAutofillGroup.id = "formAutofillGroup";
    addressAutofill.id = "addressAutofill";
    addressAutofillLearnMore.id = "addressAutofillLearnMore";

    formAutofillGroupBoxLabelHeading.textContent = this.bundle.GetStringFromName("autofillHeader");

    addressAutofill.setAttribute("data-subcategory", "address-autofill");
    addressAutofillCheckbox.setAttribute("label", this.bundle.GetStringFromName("autofillAddressesCheckbox"));
    addressAutofillLearnMore.textContent = this.bundle.GetStringFromName("learnMoreLabel");
    savedAddressesBtn.setAttribute("label", this.bundle.GetStringFromName("savedAddressesBtnLabel"));
    // Align the start to keep the savedAddressesBtn as original size
    // when addressAutofillCheckboxGroup's height is changed by a longer l10n string
    savedAddressesBtnWrapper.setAttribute("align", "start");

    addressAutofillLearnMore.setAttribute("href", learnMoreURL);

    // Add preferences search support
    savedAddressesBtn.setAttribute("searchkeywords", MANAGE_ADDRESSES_KEYWORDS.concat(EDIT_ADDRESS_KEYWORDS)
                                                       .map(key => this.bundle.GetStringFromName(key)).join("\n"));

    // Manually set the checked state
    if (FormAutofill.isAutofillAddressesEnabled) {
      addressAutofillCheckbox.setAttribute("checked", true);
    }

    addressAutofillCheckboxGroup.align = "center";
    addressAutofillCheckboxGroup.flex = 1;

    formAutofillGroupBoxLabel.appendChild(formAutofillGroupBoxLabelHeading);
    formAutofillFragment.appendChild(formAutofillGroupBoxLabel);
    formAutofillFragment.appendChild(formAutofillGroup);
    formAutofillGroup.appendChild(addressAutofill);
    addressAutofill.appendChild(addressAutofillCheckboxGroup);
    addressAutofillCheckboxGroup.appendChild(addressAutofillCheckbox);
    addressAutofillCheckboxGroup.appendChild(addressAutofillLearnMore);
    addressAutofill.appendChild(savedAddressesBtnWrapper);
    savedAddressesBtnWrapper.appendChild(savedAddressesBtn);

    this.refs = {
      formAutofillFragment,
      formAutofillGroup,
      addressAutofillCheckbox,
      savedAddressesBtn,
    };

    if (FormAutofill.isAutofillCreditCardsAvailable) {
      let creditCardAutofill = document.createXULElement("hbox");
      let creditCardAutofillCheckboxGroup = document.createXULElement("hbox");
      let creditCardAutofillCheckbox = document.createXULElement("checkbox");
      let creditCardAutofillLearnMore = document.createXULElement("label");
      let savedCreditCardsBtn = document.createXULElement("button");
      savedCreditCardsBtn.className = "accessory-button";
      creditCardAutofillCheckbox.className = "tail-with-learn-more";
      creditCardAutofillLearnMore.className = "learnMore text-link";

      creditCardAutofill.id = "creditCardAutofill";
      creditCardAutofillLearnMore.id = "creditCardAutofillLearnMore";

      creditCardAutofill.setAttribute("data-subcategory", "credit-card-autofill");
      creditCardAutofillCheckbox.setAttribute("label", this.bundle.GetStringFromName("autofillCreditCardsCheckbox"));
      creditCardAutofillLearnMore.textContent = this.bundle.GetStringFromName("learnMoreLabel");
      savedCreditCardsBtn.setAttribute("label", this.bundle.GetStringFromName("savedCreditCardsBtnLabel"));
      // Align the start to keep the savedCreditCardsBtn as original size
      // when creditCardAutofillCheckboxGroup's height is changed by a longer l10n string
      savedCreditCardsBtnWrapper.setAttribute("align", "start");

      creditCardAutofillLearnMore.setAttribute("href", learnMoreURL);

      // Add preferences search support
      savedCreditCardsBtn.setAttribute("searchkeywords", MANAGE_CREDITCARDS_KEYWORDS.concat(EDIT_CREDITCARD_KEYWORDS)
                                                           .map(key => this.bundle.GetStringFromName(key)).join("\n"));

      // Manually set the checked state
      if (FormAutofill.isAutofillCreditCardsEnabled) {
        creditCardAutofillCheckbox.setAttribute("checked", true);
      }

      creditCardAutofillCheckboxGroup.align = "center";
      creditCardAutofillCheckboxGroup.flex = 1;

      formAutofillGroup.appendChild(creditCardAutofill);
      creditCardAutofill.appendChild(creditCardAutofillCheckboxGroup);
      creditCardAutofillCheckboxGroup.appendChild(creditCardAutofillCheckbox);
      creditCardAutofillCheckboxGroup.appendChild(creditCardAutofillLearnMore);
      creditCardAutofill.appendChild(savedCreditCardsBtnWrapper);
      savedCreditCardsBtnWrapper.appendChild(savedCreditCardsBtn);

      this.refs.creditCardAutofillCheckbox = creditCardAutofillCheckbox;
      this.refs.savedCreditCardsBtn = savedCreditCardsBtn;
    }
  },

  /**
   * Handle events
   *
   * @param  {DOMEvent} event
   */
  handleEvent(event) {
    switch (event.type) {
      case "command": {
        let target = event.target;

        if (target == this.refs.addressAutofillCheckbox) {
          // Set preference directly instead of relying on <Preference>
          Services.prefs.setBoolPref(ENABLED_AUTOFILL_ADDRESSES_PREF, target.checked);
        } else if (target == this.refs.creditCardAutofillCheckbox) {
          Services.prefs.setBoolPref(ENABLED_AUTOFILL_CREDITCARDS_PREF, target.checked);
        } else if (target == this.refs.savedAddressesBtn) {
          target.ownerGlobal.gSubDialog.open(MANAGE_ADDRESSES_URL);
        } else if (target == this.refs.savedCreditCardsBtn) {
          target.ownerGlobal.gSubDialog.open(MANAGE_CREDITCARDS_URL);
        }
        break;
      }
      case "click": {
        let target = event.target;

        if (target == this.refs.addressAutofillCheckboxLabel) {
          let pref = FormAutofill.isAutofillAddressesEnabled;
          Services.prefs.setBoolPref(ENABLED_AUTOFILL_ADDRESSES_PREF, !pref);
          this.refs.addressAutofillCheckbox.checked = !pref;
        } else if (target == this.refs.creditCardAutofillCheckboxLabel) {
          let pref = FormAutofill.isAutofillCreditCardsEnabled;
          Services.prefs.setBoolPref(ENABLED_AUTOFILL_CREDITCARDS_PREF, !pref);
          this.refs.creditCardAutofillCheckbox.checked = !pref;
        }
        break;
      }
    }
  },

  /**
   * Attach event listener
   */
  attachEventListeners() {
    this.refs.formAutofillGroup.addEventListener("command", this);
    this.refs.formAutofillGroup.addEventListener("click", this);
  },

  /**
   * Remove event listener
   */
  detachEventListeners() {
    this.refs.formAutofillGroup.removeEventListener("command", this);
    this.refs.formAutofillGroup.removeEventListener("click", this);
  },
};
PK