/*
 * Decompiled with CFR 0.152.
 */
class kParameterXSPLINE
extends kParameterX {
    kParameterXSPLINE() {
    }

    public Object Out() {
        return null;
    }

    public double doubleout(double d) {
        return this.spltable.spline(d - this.a);
    }
}

