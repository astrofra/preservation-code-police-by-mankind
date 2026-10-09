// 
// Decompiled by Procyon v0.6.0
// 

class kSoundCommand
{
    int commandcode;
    kSound snd;
    
    public void MakeSoundCommand() {
        if (this.commandcode == 0) {
            this.snd.StopOne();
        }
        if (this.commandcode == 1) {
            this.snd.PlayOne();
        }
        if (this.commandcode == 2) {
            this.snd.LoopOne();
        }
    }
}
