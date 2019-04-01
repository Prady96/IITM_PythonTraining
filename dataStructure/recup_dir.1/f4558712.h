 > option {
  display: flex;
  align-items: center;
  height: 1.6em;
  padding-inline-start: 0.6em;
}

#controls-container {
  flex: 0 1 100%;
  justify-content: end;
  margin-top: 1em;
}

#remove {
  margin-inline-start: 0;
  margin-inline-end: auto;
}

#edit {
  margin-inline-end: 0;
}

#credit-cards > option::before {
  content: "";
  background: url("icon-credit-card-generic.svg") no-repeat;
  background-size: contain;
  float: left;
  width: 16px;
  height: 16px;
  padding-inline-end: 10px;
}

/*
  We use .png / @2x.png images where we don't yet have a vector version of a logo
*/
#credit-cards.branded > option[cc-type="amex"]::before {
  background-image: url("third-party/cc-logo-amex.png");
}

#credit-cards.branded > option[cc-type="cartebancaire"]::before {
  background-image: url("third-party/cc-logo-cartebancaire.png");
}

#credit-cards.branded > option[cc-type="diners"]::before {
  background-image: url("third-party/cc-logo-diners.svg");
}

#credit-cards.branded > option[cc-type="discover"]::before {
  background-image: url("third-party/cc-logo-discover.png");
}

#credit-cards.branded > option[cc-type="jcb"]::before {
  background-image: url("third-party/cc-logo-jcb.svg");
}

#credit-cards.branded > option[cc-type="mastercard"]::before {
  background-image: url("third-party/cc-logo-mastercard.svg");
}

#credit-cards.branded > option[cc-type="mir"]::before {
  background-image: url("third-party/cc-logo-mir.svg");
}

#credit-cards.branded > option[cc-type="unionpay"]::before {
  background-image: url("third-party/cc-logo-unionpay.svg");
}

#credit-cards.branded > option[cc-type="visa"]::before {
  background-image: url("third-party/cc-logo-visa.svg");
}

@media (min-resolution: 1.1dppx) {
  #credit-cards.branded > option[cc-type="amex"]::before {
    background-image: url("third-party/cc-logo-amex@2x.png");
  }
  #credit-cards.branded > option[cc-type="cartebancaire"]::before {
    background-image: url("third-party/cc-logo-cartebancaire@2x.png");
  }
  #credit-cards.branded > option[cc-type="discover"]::before {
    background-image: url("third-party/cc-logo-discover@2x.png");
  }
}
PK