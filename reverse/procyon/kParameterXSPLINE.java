// 
// Decompiled by Procyon v0.6.0
// 

class kParameterXSPLINE extends kParameterX
{
    public Object Out() {
        return null;
    }
    
    public double doubleout(final double n) {
        return super.spltable.spline(n - super.a);
    }
}
