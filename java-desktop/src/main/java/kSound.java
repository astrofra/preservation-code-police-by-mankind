/*
 * Decompiled with CFR 0.152.
 */


public class kSound {
    private DemoAudio OurAudio;
    public boolean soundIsOn;
    public boolean playedonce;
    static kSound firstkSound = null;
    private kSound nextkSound = null;

    public kSound(DemoAudio audioClip) {
        this.OurAudio = audioClip;
        this.soundIsOn = false;
        this.nextkSound = firstkSound;
        firstkSound = this;
    }

    public void LoopOne() {
        if (this.OurAudio != null) {
            this.OurAudio.loop();
            this.soundIsOn = true;
        }
    }

    public void PlayOne() {
        if (this.OurAudio != null) {
            this.OurAudio.play();
            this.soundIsOn = false;
            this.playedonce = true;
        }
    }

    public static void StopAll() {
        kSound kSound2 = firstkSound;
        while (kSound2 != null) {
            kSound2.StopOne();
            kSound2 = kSound2.nextkSound;
        }
    }

    public void StopOne() {
        if (this.OurAudio != null) {
            this.OurAudio.stop();
            this.soundIsOn = false;
        }
    }
}
