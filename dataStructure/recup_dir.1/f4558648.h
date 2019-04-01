a.org/keymaster/gatekeeper/there.is.only.xul"
          xmlns:xbl="http://www.mozilla.org/xbl">

  <binding id="autocomplete-profile-listitem-base" extends="chrome://global/content/bindings/richlistbox.xml#richlistitem">
    <implementation implements="nsIDOMXULSelectControlItemElement">
      <constructor>
      </constructor>
      <!-- For form autofill, we want to unify the selection no matter by
      keyboard navigation or mouseover in order not to confuse user which
      profile preview is being shown. This field is set to true to indicate
      that selectedIndex of popup should be changed while mouseover item -->
      <field name="selectedByMouseOver">true</field>

      <property name="_stringBundle">
        <getter><![CDATA[
          /* global Services */
          if (!this.__stringBundle) {
            this.__stringBundle = Services.strings.createBundle("chrome://formautofill/locale/formautofill.properties");
          }
          return this.__stringBundle;
        ]]></getter>
      </property>

      <method name="_cleanup">
        <body>
        <![CDATA[
          this.removeAttribute("formautofillattached");
          if (this._itemBox) {
            this._itemBox.removeAttribute("size");
          }
        ]]>
        </body>
      </method>

      <method name="_onOverflow">
        <body></body>
      </method>

      <method name="_onUnderflow">
        <body></body>
      </method>

      <method name="handleOverUnderflow">
        <body></body>
      </method>

      <method name="_adjustAutofillItemLayout">
        <body>
        <![CDATA[
          let outerBoxRect = this.parentNode.getBoundingClientRect();

          // Make item fit in popup as XUL box could not constrain
          // item's width
          this._itemBox.style.width = outerBoxRect.width + "px";
          // Use two-lines layout when width is smaller than 150px or
          // 185px if an image precedes the label.
          let oneLineMinRequiredWidth = this.getAttribute("ac-image") ? 185 : 150;

          if (outerBoxRect.width <= oneLineMinRequiredWidth) {
            this._itemBox.setAttribute("size", "small");
          } else {
            this._itemBox.removeAttribute("size");
          }
        ]]>
        </body>
      </method>
    </implementation>
  </binding>

  <binding id="autocomplete-profile-listitem" extends="chrome://formautofill/content/formautofill.xml#autocomplete-profile-listitem-base">
    <xbl:content xmlns="http://www.w3.org/1999/xhtml">
      <div anonid="autofill-item-box" class="autofill-item-box" xbl:inherits="ac-image">
        <div class="profile-label-col profile-item-col">
          <span anonid="profile-label-affix" class="profile-label-affix"></span>
          <span anonid="profile-label" class="profile-label"></span>
        </div>
        <div class="profile-comment-col profile-item-col">
          <span anonid="profile-comment" class="profile-comment"></span>
        </div>
      </div>
    </xbl:content>

    <implementation implements="nsIDOMXULSelectControlItemElement">
      <constructor>
        <![CDATA[
          this._itemBox = document.getAnonymousElementByAttribute(
            this, "anonid", "autofill-item-box"
          );
          this._labelAffix = document.getAnonymousElementByAttribute(
            this, "anonid", "profile-label-affix"
          );
          this._label = document.getAnonymousElementByAttribute(
            this, "anonid", "profile-label"
          );
          this._comment = document.getAnonymousElementByAttribute(
            this, "anonid", "profile-comment"
          );

          this._adjustAcItem();
        ]]>
      </constructor>

      <property name="selected" onget="return this.getAttribute('selected') == 'true';">
        <setter><![CDATA[
          /* global Cu */
          if (val) {
            this.setAttribute("selected", "true");
          } else {
            this.removeAttribute("selected");
          }

          let {AutoCompletePopup} = ChromeUtils.import("resource://gre/modules/AutoCompletePopup.jsm", {});

          AutoCompletePopup.sendMessageToBrowser("FormAutofill:PreviewProfile");

          return val;
        ]]></setter>
      </property>

      <method name="_adjustAcItem">
        <body>
        <![CDATA[
          this._adjustAutofillItemLayout();
          this.setAttribute("formautofillattached", "true");
          this._itemBox.style.setProperty("--primary-icon", `url(${this.getAttribute("ac-image")})`);

          let {primaryAffix, primary, secondary} = JSON.parse(this.getAttribute("ac-value"));

          this._labelAffix.textContent = primaryAffix;
          this._label.textContent = primary;
          this._comment.textContent = secondary;
        ]]>
        </body>
      </method>
    </implementation>
  </binding>

  <binding id="autocomplete-profile-listitem-footer" extends="chrome://formautofill/content/formautofill.xml#autocomplete-profile-listitem-base">
    <xbl:content xmlns="http://www.w3.org/1999/xhtml">
      <div anonid="autofill-footer" class="autofill-item-box autofill-footer">
        <div anonid="autofill-warning" class="autofill-footer-row autofill-warning">
        </div>
        <div anonid="autofill-option-button" class="autofill-footer-row autofill-button">
        </div>
      </div>
    </xbl:content>

    <handlers>
      <handler event="click" button="0"><![CDATA[
        if (this._warningTextBox.contains(event.originalTarget)) {
          return;
        }

        window.openPreferences("privacy-form-autofill", {origin: "autofillFooter"});
      ]]></handler>
    </handlers>

    <implementation implements="nsIDOMXULSelectControlItemElement">
      <constructor>
        <![CDATA[
          this._itemBox = document.getAnonymousElementByAttribute(
            this, "anonid", "autofill-footer"
          );
          this._optionButton = document.getAnonymousElementByAttribute(
            this, "anonid", "autofill-option-button"
          );
          this._warningTextBox = document.getAnonymousElementByAttribute(
            this, "anonid", "autofill-warning"
          );

          /**
           * A handler for updating warning message once selectedIndex has been changed.
           *
           * There're three different states of warning message:
           * 1. None of addresses were selected: We show all the categories intersection of fields in the
           *    form and fields in the results.
           * 2. An address was selested: Show the additional categories that will also be filled.
           * 3. An address was selected, but the focused category is the same as the only one category: Only show
           * the exact category that we're going to fill in.
           *
           * @private
           * @param {string[]} data.categories
           *        The categories of all the fields contained in the selected address.
           */
          this._updateWarningNote = ({data} = {}) => {
            let categories = (data && data.categories) ? data.categories : this._allFieldCategories;
            // If the length of categories is 1, that means all the fillable fields are in the same
            // category. We will change the way to inform user according to this flag. When the value
            // is true, we show "Also autofills ...", otherwise, show "Autofills ..." only.
            let hasExtraCategories = categories.length > 1;
            // Show the categories in certain order to conform with the spec.
            let orderedCategoryList = [{id: "address", l10nId: "category.address"},
                                       {id: "name", l10nId: "category.name"},
                                       {id: "organization", l10nId: "category.organization2"},
                                       {id: "tel", l10nId: "category.tel"},
                                       {id: "email", l10nId: "category.email"}];
            let showCategories = hasExtraCategories ?
              orderedCategoryList.filter(category => categories.includes(category.id) && category.id != this._focusedCategory) :
              [orderedCategoryList.find(category => category.id == this._focusedCategory)];

            let separator = this._stringBundle.GetStringFromName("fieldNameSeparator");
            let warningTextTmplKey = hasExtraCategories ? "phishingWarningMessage" : "phishingWarningMessage2";
            let categoriesText = showCategories.map(category => this._stringBundle.GetStringFromName(category.l10nId)).join(separator);

            this._warningTextBox.textContent = this._stringBundle.formatStringFromName(warningTextTmplKey,
              [categoriesText], 1);
            this.parentNode.parentNode.adjustHeight();
          };

          this._adjustAcItem();
        ]]>
      </constructor>

      <method name="_onCollapse">
        <body>
        <![CDATA[
          /* global messageManager */

          if (this.showWarningText) {
            messageManager.removeMessageListener("FormAutofill:UpdateWarningMessage", this._updateWarningNote);
          }

          this._itemBox.removeAttribute("no-warning");
        ]]>
        </body>
      </method>

      <method name="_adjustAcItem">
        <body>
        <![CDATA[
          /* global Cu */
          this._adjustAutofillItemLayout();
          this.setAttribute("formautofillattached", "true");

          let {AppConstants} = ChromeUtils.import("resource://gre/modules/AppConstants.jsm", {});
          // TODO: The "Short" suffix is pointless now as normal version string is no longer needed,
          // we should consider removing the suffix if possible when the next time locale change.
          let buttonTextBundleKey = AppConstants.platform == "macosx" ?
            "autocompleteFooterOptionOSXShort" : "autocompleteFooterOptionShort";
          let buttonText = this._stringBundle.GetStringFromName(buttonTextBundleKey);
          this._optionButton.textContent = buttonText;

          let value = JSON.parse(this.getAttribute("ac-value"));

          this._allFieldCategories = value.categories;
          this._focusedCategory = value.focusedCategory;
          this.showWarningText = this._allFieldCategories && this._focusedCategory;

          if (this.showWarningText) {
            messageManager.addMessageListener("FormAutofill:UpdateWarningMessage", this._updateWarningNote);

            this._updateWarningNote();
          } else {
            this._itemBox.setAttribute("no-warning", "true");
          }
        ]]>
        </body>
      </method>
    </implementation>
  </binding>

  <binding id="autocomplete-creditcard-insecure-field" extends="chrome://formautofill/content/formautofill.xml#autocomplete-profile-listitem-base">
    <xbl:content xmlns="http://www.w3.org/1999/xhtml">
      <div anonid="autofill-item-box" class="autofill-insecure-item">
      </div>
    </xbl:content>

    <implementation implements="nsIDOMXULSelectControlItemElement">
      <constructor>
      <![CDATA[
        this._itemBox = document.getAnonymousElementByAttribute(
          this, "anonid", "autofill-item-box"
        );

        this._adjustAcItem();
      ]]>
      </constructor>

      <property name="selected" onget="return this.getAttribute('selected') == 'true';">
        <setter><![CDATA[
          // Make this item unselectable since we see this item as a pure message.
          return false;
        ]]></setter>
      </property>

      <method name="_adjustAcItem">
        <body>
        <![CDATA[
          this._adjustAutofillItemLayout();
          this.setAttribute("formautofillattached", "true");

          let value = this.getAttribute("ac-value");
          this._itemBox.textContent = value;
        ]]>
        </body>
      </method>

    </implementation>
  </binding>

  <binding id="autocomplete-profile-listitem-clear-button" extends="chrome://formautofill/content/formautofill.xml#autocomplete-profile-listitem-base">
    <xbl:content xmlns="http://www.w3.org/1999/xhtml">
      <div anonid="autofill-item-box" class="autofill-item-box autofill-footer">
        <div anonid="autofill-clear-button" class="autofill-footer-row autofill-button"></div>
      </div>
    </xbl:content>

    <handlers>
      <handler event="click" button="0"><![CDATA[
        /* global Cu */
        let {AutoCompletePopup} = ChromeUtils.import("resource://gre/modules/AutoCompletePopup.jsm", {});

        AutoCompletePopup.sendMessageToBrowser("FormAutofill:ClearForm");
      ]]></handler>
    </handlers>

    <implementation implements="nsIDOMXULSelectControlItemElement">
      <constructor>
      <![CDATA[
        this._itemBox = document.getAnonymousElementByAttribute(
          this, "anonid", "autofill-item-box"
        );
        this._clearBtn = document.getAnonymousElementByAttribute(
          this, "anonid", "autofill-clear-button"
        );

        this._adjustAcItem();
      ]]>
      </constructor>

      <method name="_adjustAcItem">
        <body>
        <![CDATA[
          this._adjustAutofillItemLayout();
          this.setAttribute("formautofillattached", "true");

          let clearFormBtnLabel = this._stringBundle.GetStringFromName("clearFormBtnLabel2");
          this._clearBtn.textContent = clearFormBtnLabel;
        ]]>
        </body>
      </method>

    </implementation>
  </binding>

</bindings>
PK