/*
 * Decompiled with CFR 0.152.
 */
class kTimer {
    private long StartTime = System.currentTimeMillis();

    public int getTime() {
        return (int)((System.currentTimeMillis() - this.StartTime) / 10L);
    }

    public void reset() {
        this.StartTime = System.currentTimeMillis();
    }
}

