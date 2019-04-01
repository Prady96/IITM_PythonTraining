n: 0;
  padding-bottom: 5px;
}

form label,
form div {
  /* Positioned so that the .label-text and .error-text children will be
     positioned relative to this. */
  position: relative;
  display: block;
  line-height: 1em;
}

form :-moz-any(label, div) .label-text {
  position: absolute;
  color: GrayText;
  pointer-events: none;
  left: 10px;
  top: .2em;
  transition: top .2s var(--animation-easing-function),
              font-size .2s var(--animation-easing-function);
}

form :-moz-any(label, div):focus-within .label-text,
form :-moz-any(label, div) .label-text[field-populated] {
  top: 0;
  font-size: var(--in-field-label-size);
}

form :-moz-any(input, select, textarea):focus ~ .label-text {
  color: var(--in-content-item-selected);
}

/* Focused error fields should get a darker text but not the blue one since it
 * doesn't look good with the red error outline. */
form :-moz-any(input, select, textarea):focus:-moz-ui-invalid ~ .label-text {
  color: var(--in-content-text-color);
}

form div[required] > label .label-text::after,
form :-moz-any(label, div)[required] .label-text::after {
  content: attr(fieldRequiredSymbol);
}

.persist-checkbox label {
  display: flex;
  flex-direction: row;
  align-items: center;
  margin-top: var(--grid-column-row-gap);
  margin-bottom: var(--grid-column-row-gap);
}

:root[subdialog] form {
  /* Match the margin-inline-start of the #controls-container buttons
     and provide enough padding at the top of the form so button outlines
     don't get clipped. */
  padding: 4px 4px 0;
}

#controls-container {
  flex: 0 1 100%;
  justify-content: end;
  margin: 1em 0 0;
}
PK