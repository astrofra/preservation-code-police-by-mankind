/*
 * Decompiled with CFR 0.152.
 */
import java.applet.AudioClip;

public class kraycasting.kSound {
    private AudioClip OurAudio;
    private boolean soundIsOn;

    public kraycasting.kSound(String string) {
        this.OurAudio = kraycasting.this.getAudioClip(kraycasting.this.getDocumentBase(), string);
        this.soundIsOn = false;
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
        }
    }

    public void StopOne() {
        if (this.OurAudio != null) {
            this.OurAudio.stop();
            this.soundIsOn = false;
        }
    }
}
