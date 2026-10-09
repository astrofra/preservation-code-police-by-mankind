// 
// Decompiled by Procyon v0.6.0
// 

class kTimer
{
    private long StartTime;
    
    public kTimer() {
        this.StartTime = System.currentTimeMillis();
    }
    
    public int getTime() {
        return (int)((System.currentTimeMillis() - this.StartTime) / 10L);
    }
    
    public void reset() {
        this.StartTime = System.currentTimeMillis();
    }
}
