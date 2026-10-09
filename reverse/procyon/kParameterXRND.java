// 
// Decompiled by Procyon v0.6.0
// 

class kParameterXRND extends kParameterX
{
    public Object Out() {
        return null;
    }
    
    public double doubleout(final double n) {
        return super.a + Math.random() * (super.b - super.a);
    }
}
