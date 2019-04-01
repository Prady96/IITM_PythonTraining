-within input:-moz-ui-invalid {
  border-color: transparent;
}

#given-name-container,
#additional-name-container,
#family-name-container {
  display: flex;
  /* The 3 pieces inside the name container don't have the .container class so
     need to set flex-grow themselves. See `.editAddressForm .container` */
  flex-grow: 1;
  /* Remove the bottom margin from the name containers so that the outer
     #name-container provides the margin on the outside */
  margin-bottom: 0 !important;
  margin-left: 0;
  margin-right: 0;
}

/* The name fields are placed adjacent to each other.
   Remove the border-radius on adjacent fields. */
#given-name:dir(ltr),
#family-name:dir(rtl) {
  border-top-right-radius: 0;
  border-bottom-right-radius: 0;
  border-right-width: 0;
}

#given-name:dir(rtl),
#family-name:dir(ltr) {
  border-top-left-radius: 0;
  border-bottom-left-radius: 0;
  border-left-width: 0;
}

#additional-name {
  border-radius: 0;
  /* This provides the inner separators between the fields and should never
   * change to the focused color. */
  border-left-color: var(--in-content-box-border-color) !important;
  border-right-color: var(--in-content-box-border-color) !important;
}

/* Since the name fields are adjacent, there isn't room for the -moz-ui-invalid
   box-shadow so raise invalid name fields and their labels above the siblings
   so the shadow is shown around all 4 sides. */
#name-container input:-moz-ui-invalid,
#name-container input:-moz-ui-invalid ~ .label-text {
  z-index: 1;
}

/* End name field rules */

#name-container,
#street-address-container {
  /* Name and street address are always full-width */
  flex: 0 1 100%;
}

#street-address {
  resize: vertical;
}

#country-warning-message {
  box-sizing: border-box;
  font-size: 1rem;
  align-items: center;
  text-align: start;
  color: #737373;
  padding-inline-start: 1em;
}

:root:not([subdialog]) #country-warning-message {
  display: none;
}
PK