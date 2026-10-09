/*
 * Decompiled with CFR 0.152.
 */
class kSoundCommand {
    int commandcode;
    kSound snd;

    kSoundCommand() {
    }

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

