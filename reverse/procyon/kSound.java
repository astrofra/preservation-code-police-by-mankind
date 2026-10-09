import java.applet.AudioClip;

// 
// Decompiled by Procyon v0.6.0
// 

public class kSound
{
    private AudioClip OurAudio;
    public boolean soundIsOn;
    public boolean playedonce;
    static kSound firstkSound;
    private kSound nextkSound;
    
    static {
        kSound.firstkSound = null;
    }
    
    public kSound(final AudioClip ourAudio) {
        this.nextkSound = null;
        this.OurAudio = ourAudio;
        this.soundIsOn = false;
        this.nextkSound = kSound.firstkSound;
        kSound.firstkSound = this;
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
        for (kSound kSound = kSound.firstkSound; kSound != null; kSound = kSound.nextkSound) {
            kSound.StopOne();
        }
    }
    
    public void StopOne() {
        if (this.OurAudio != null) {
            this.OurAudio.stop();
            this.soundIsOn = false;
        }
    }
}
