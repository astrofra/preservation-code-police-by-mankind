// Original expression and call order retained; platform changes documented.
#include "engine.hpp"

// kraycasting.FindHowMany — reconstructed Java line 30
jint kraycasting::FindHowMany(String string, String string2) {
    jint n = jint(1);
    jint n2 = string->length();
    jint n3 = jint(0);
    jint n4 = jint(0);
    while (((n3 != (-jint(1))) && (n4 < n2)))
    {
        (n3 = string->indexOf(string2, n4));
        if ((n3 == (-jint(1))))
        continue;
        (++n);
        (n4 = (n3 + jint(1)));
    }
    return n;
}

// kraycasting.FindInto — reconstructed Java line 44
String kraycasting::FindInto(String string, jint n, String string2) {
    jint n2 = jint(2);
    jint n3 = jint(1);
    jint n4 = string->length();
    jint n5 = jint(0);
    jint n6 = jint(0);
    while (((n5 != (-jint(1))) && (n6 < n4)))
    {
        (n5 = string->indexOf(string2, n6));
        if ((n5 == (-jint(1))))
        continue;
        (++n2);
        (n6 = (n5 + jint(1)));
    }
    Array<jint> nArray = Array<jint>(n2);
    (nArray[jint(0)] = (-jint(1)));
    (nArray[(n2 - jint(1))] = n4);
    (n6 = jint(0));
    (n5 = jint(0));
    while (((n5 != (-jint(1))) && (n6 < n4)))
    {
        (n5 = string->indexOf(string2, n6));
        if ((n5 == (-jint(1))))
        continue;
        (nArray[n3] = n5);
        (++n3);
        (n6 = (n5 + jint(1)));
    }
    return string->substring((nArray[n] + jint(1)), nArray[(n + jint(1))]);
}

// kraycasting.SplitSTag — reconstructed Java line 71
Array<String> kraycasting::SplitSTag(String string, String string2) {
    String string3 = ((String("<") + string2) + String(">"));
    String string4 = ((String("</") + string2) + String(">"));
    jint n = jint(0);
    jint n2 = string->length();
    jint n3 = jint(0);
    jint n4 = jint(0);
    jint n5 = jint(0);
    while (((n5 != (-jint(1))) && (n4 < n2)))
    {
        (n5 = string->indexOf(string3, n4));
        if ((n5 == (-jint(1))))
        continue;
        (++n3);
        (n4 = (n5 + string3->length()));
    }
    if ((n3 == jint(0)))
    {
        return nullptr;
    }
    Array<String> stringArray = Array<String>(n3);
    (n3 = jint(0));
    (n4 = jint(0));
    (n5 = jint(0));
    while (((n5 != (-jint(1))) && (n4 < n2)))
    {
        (n5 = string->indexOf(string3, n4));
        if ((n5 == (-jint(1))))
        continue;
        (n4 = (n5 + string3->length()));
        (n = string->indexOf(string4, n4));
        (stringArray[n3] = string->substring(n4, n));
        (++n3);
    }
    return stringArray;
}

// kraycasting.init — reconstructed Java line 107
void kraycasting::init() {
    jint n{};
    jint n2{};
    (this->RenderContext = std::make_shared<kRenderContext>(this, this));
    (this->applet_image = this->RenderContext->createImage());
    (this->THEkScript = std::make_shared<kScript>(this, this->getParameter(String("SCRIPT"))));
    (this->rcs = std::make_shared<raycasting>(this, this->RenderContext));
    (this->ct = Array<int8_t>(jint(8192)));
    jint n3 = jint(0);
    while ((n3 < jint(64)))
    {
        (n2 = jint(0));
        while ((n2 < jint(64)))
        {
            (n = jinteger(((n2 * (n3 + jint(1))) >> jint(6))));
            (this->ct[((n3 << jint(6)) | n2)] = n->byteValue());
            (++n2);
        }
        (++n3);
    }
    (n3 = jint(64));
    while ((n3 < jint(128)))
    {
        (n2 = jint(0));
        while ((n2 < jint(64)))
        {
            (n = jinteger((n2 + (((jint(63) - n2) * (n3 - jint(64))) >> jint(6)))));
            (this->ct[((n3 << jint(6)) | n2)] = n->byteValue());
            (++n2);
        }
        (++n3);
    }
    (this->kTime = std::make_shared<kTimer>(this));
}

// kTexture.kTexture — reconstructed Java line 188
kTexture::kTexture(kraycasting* owner, Ref<Image> image) : EngineObject(owner) {
    (this->Modulo = image->width);
    (this->Height = image->height);
    (this->pixels = image->pixels);
}

// kRenderContext.kRenderContext — reconstructed Java line 286
kRenderContext::kRenderContext(kraycasting* owner, kraycasting* applet) : EngineObject(owner) {
    jint n = jint(0);
    jint n2 = jint(0);
    jint n3 = jint(0);
    (this->app = applet);
    (this->ClipX1 = jint(0));
    (this->ClipY1 = jint(0));
    (this->ClipX2 = applet->getSize()->width);
    (this->ClipY2 = applet->getSize()->height);
    jint n4 = (this->ClipX2 - this->ClipX1);
    jint n5 = (this->ClipY2 - this->ClipY1);
    (this->drawable = true);
    double d = (1.0 / double(n4));
    double d2 = (1.0 / double(n5));
    (this->Modulo = this->ClipX2);
    (this->Height = this->ClipY2);
    (this->Chunk2 = Array<jint>((this->Modulo * this->Height)));
    (this->CurrentChunk = Array<jint>((this->Modulo * this->Height)));
    (this->RC_RootContext = raw(this));
    (this->MaxX = this->Modulo);
    (this->MaxY = this->Height);
    (this->MaxPreX = ((this->MaxX >> jint(4)) + jint(2)));
    (this->MaxPreY = ((this->MaxY >> jint(4)) + jint(2)));
    (this->Pretable = array2<Ref<kPixel>>(this->MaxPreX, this->MaxPreY));
    jint n6 = (n4 >> jint(1));
    jint n7 = (n5 >> jint(1));
    (n2 = jint(16));
    jint n8 = jint(0);
    jint n9 = jint(0);
    while ((n9 < (n5 + n2)))
    {
        if (((n9 + jint(16)) > n5))
        {
            (n2 = (n5 - n9));
        }
        (n3 = jint(16));
        (n = jint(0));
        jint n10 = jint(0);
        while ((n10 < (n4 + n3)))
        {
            if (((n10 + jint(16)) > n4))
            {
                (n3 = (n4 - n10));
            }
            (this->Pretable[n][n8] = std::make_shared<kPixel>(owner));
            (this->Pretable[n][n8]->XI = (double((n10 - n6)) * d));
            (this->Pretable[n][n8]->YI = (double((n9 - n7)) * d2));
            (++n);
            (n10 += n3);
        }
        (++n8);
        (n9 += n2);
    }
    (this->MaxPreX = n);
    (this->MaxPreY = n8);
}

// kRenderContext.kRenderContext — reconstructed Java line 286
kRenderContext::kRenderContext(kraycasting* owner, Ref<kRenderContext> kRenderContext2) : EngineObject(owner) {
    (this->ClipX1 = kRenderContext2->ClipX1);
    (this->ClipY1 = kRenderContext2->ClipY1);
    (this->ClipX2 = kRenderContext2->ClipX2);
    (this->ClipY2 = kRenderContext2->ClipY2);
    (this->Modulo = kRenderContext2->Modulo);
    (this->Height = kRenderContext2->Height);
    (this->app = kRenderContext2->app);
    (this->RC_RootContext = raw(kRenderContext2));
    (this->RC_FatherContext = raw(kRenderContext2));
    (this->drawable = kRenderContext2->drawable);
}

// kRenderContext.kRenderContext — reconstructed Java line 299
kRenderContext::kRenderContext(kraycasting* owner, Ref<kRenderContext> kRenderContext2, double d, double d2, double d3, double d4) : EngineObject(owner) {
    (this->RC_RootContext = raw(kRenderContext2->RC_RootContext));
    (this->Pretable = array2<Ref<kPixel>>(this->RC_RootContext->MaxPreX, this->RC_RootContext->MaxPreY));
    this->SetRect(kRenderContext2, d, d2, d3, d4);
}

// kRenderContext.SetRect — reconstructed Java line 305
void kRenderContext::SetRect(Ref<kRenderContext> kRenderContext2, double d, double d2, double d3, double d4) {
    jint n = jint(0);
    jint n2 = jint(0);
    jint n3 = jint(0);
    double d5 = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    double d6 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    (this->drawable = true);
    if ((d < 0.0))
    {
        (d = 0.0);
    }
    if ((d2 < 0.0))
    {
        (d2 = 0.0);
    }
    if ((d3 > 1.0))
    {
        (d3 = 1.0);
    }
    if ((d4 > 1.0))
    {
        (d4 = 1.0);
    }
    (this->ClipX1 = (kRenderContext2->ClipX1 + jint((d5 * d))));
    (this->ClipY1 = (kRenderContext2->ClipY1 + jint((d6 * d2))));
    (this->ClipX2 = (kRenderContext2->ClipX1 + jint((d5 * d3))));
    (this->ClipY2 = (kRenderContext2->ClipY1 + jint((d6 * d4))));
    (this->Modulo = kRenderContext2->Modulo);
    (this->Height = kRenderContext2->Height);
    (this->app = kRenderContext2->app);
    jint n4 = (this->ClipX2 - this->ClipX1);
    jint n5 = (this->ClipY2 - this->ClipY1);
    double d7 = (1.0 / double(n4));
    double d8 = (1.0 / double(n5));
    if (((((((n4 <= jint(0)) || (n5 <= jint(0))) || (this->ClipX1 < kRenderContext2->ClipX1)) || (this->ClipY1 < kRenderContext2->ClipY1)) || (this->ClipX2 > kRenderContext2->ClipX2)) || (this->ClipY2 > kRenderContext2->ClipY2)))
    {
        (this->drawable = false);
        return;
    }
    (this->MaxX = n4);
    (this->MaxY = n5);
    (this->MaxPreX = ((this->MaxX >> jint(4)) + jint(2)));
    (this->MaxPreY = ((this->MaxY >> jint(4)) + jint(2)));
    jint n6 = (n4 >> jint(1));
    jint n7 = (n5 >> jint(1));
    (n2 = jint(16));
    jint n8 = jint(0);
    jint n9 = jint(0);
    while ((n9 < (n5 + n2)))
    {
        if (((n9 + jint(16)) > n5))
        {
            (n2 = (n5 - n9));
        }
        (n3 = jint(16));
        (n = jint(0));
        jint n10 = jint(0);
        while ((n10 < (n4 + n3)))
        {
            if (((n10 + jint(16)) > n4))
            {
                (n3 = (n4 - n10));
            }
            (this->Pretable[n][n8] = std::make_shared<kPixel>(owner));
            (this->Pretable[n][n8]->XI = (double((n10 - n6)) * d7));
            (this->Pretable[n][n8]->YI = (double((n9 - n7)) * d8));
            (++n);
            (n10 += n3);
        }
        (++n8);
        (n9 += n2);
    }
    (this->MaxPreX = n);
    (this->MaxPreY = n8);
}

// kRenderContext.SetSub — reconstructed Java line 372
void kRenderContext::SetSub(Ref<kRenderContext> kRenderContext2, jint n, jint n2, jint n3, jint n4) {
    (this->ClipX1 = n);
    (this->ClipY1 = n2);
    (this->ClipX2 = n3);
    (this->ClipY2 = n4);
    (this->CurrentChunk = kRenderContext2->CurrentChunk);
    (this->RC_RootContext = raw(kRenderContext2->RC_RootContext));
}

// kRenderContext.createImage — reconstructed Java line 381
Array<jint> kRenderContext::createImage() {
    Array<jint> nArray = this->CurrentChunk;
    (this->CurrentChunk = this->Chunk2);
    (this->Chunk2 = nArray);
    return this->Chunk2;
}

// kcamera.kcamera — reconstructed Java line 401
kcamera::kcamera(kraycasting* owner, double d, double d2, double d3, double d4, double d5, double d6, double d7) : EngineObject(owner) {
    (this->X = d);
    (this->Y = d2);
    (this->Z = d3);
    (this->FOV = d7);
    (this->O1 = d4);
    (this->O2 = d5);
    (this->O3 = d6);
}

// kcamera.set — reconstructed Java line 411
void kcamera::set(double d, double d2, double d3, double d4, double d5, double d6, double d7) {
    (this->X = d);
    (this->Y = d2);
    (this->Z = d3);
    (this->FOV = d7);
    (this->O1 = d4);
    (this->O2 = d5);
    (this->O3 = d6);
}

// kcamera.target — reconstructed Java line 421
void kcamera::target(double d, double d2, double d3, double d4, double d5, double d6, double d7, double d8) {
    (this->X = d);
    (this->Y = d2);
    (this->Z = d3);
    (this->FOV = d7);
    (this->O1 = d8);
    (d4 -= d);
    (d5 -= d2);
    if (((d6 -= d3) == 0.0))
    {
        (d6 = 1.0E-6);
    }
    double d9 = (d4 / d6);
    (d9 = std::atan(d9));
    if ((d6 > 0.0))
    {
        (d9 += 3.141592653589793);
    }
    (this->O3 = (d9 += 3.141592653589793));
    double d10 = ((d6 * std::cos(d9)) + (d4 * std::sin(d9)));
    if ((d10 == 0.0))
    {
        (d10 = 1.0E-6);
    }
    (d9 = (d5 / d10));
    (d9 = std::atan(d9));
    if ((d10 > 0.0))
    {
        (d9 += 3.141592653589793);
    }
    (this->O2 = (-(d9 += 3.141592653589793)));
}

// kparticle.kparticle — reconstructed Java line 467
kparticle::kparticle(kraycasting* owner, jint n, double d, double d2, double d3, double d4, double d5, double d6, double d7, double d8) : EngineObject(owner) {
    if ((n == jint(0)))
    {
        (n = jint(1));
    }
    (this->nbparticle = n);
    (this->posx = Array<double>(n));
    (this->posy = Array<double>(n));
    (this->posz = Array<double>(n));
    (this->Vx = Array<double>(n));
    (this->Vy = Array<double>(n));
    (this->Vz = Array<double>(n));
    (this->Ax = Array<double>(n));
    (this->Ay = Array<double>(n));
    (this->Az = Array<double>(n));
    (this->force = Array<double>(n));
    this->reset(d, d2, d3, d4, d5, d6, d7, d8);
}

// kparticle.drawflare — reconstructed Java line 485
void kparticle::drawflare(Ref<kcamera> kcamera2, Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, double d, double d2) {
    double d3 = std::cos(kcamera2->O1);
    double d4 = std::sin(kcamera2->O1);
    double d5 = std::cos(kcamera2->O2);
    double d6 = std::sin(kcamera2->O2);
    double d7 = std::cos(kcamera2->O3);
    double d8 = std::sin(kcamera2->O3);
    jint n = jint(0);
    while ((n < this->nbparticle))
    {
        double d9 = (this->posx[n] - kcamera2->X);
        double d10 = (this->posy[n] - kcamera2->Y);
        double d11 = (this->posz[n] - kcamera2->Z);
        double d12 = (((-d8) * d11) + (d7 * d9));
        double d13 = d10;
        double d14 = ((d7 * d11) + (d8 * d9));
        double d15 = d12;
        double d16 = ((d5 * d13) + (d6 * d14));
        double d17 = (((-d6) * d13) + (d5 * d14));
        (d9 = ((d3 * d15) + (d4 * d16)));
        (d10 = (((-d4) * d15) + (d3 * d16)));
        (d11 = d17);
        owner->rcs->flare3D(d9, d10, d11, d, d2, kcamera2->FOV, kRenderContext2, kTexture2);
        (++n);
    }
}

// kparticle.drawsprit — reconstructed Java line 511
void kparticle::drawsprit(Ref<kcamera> kcamera2, Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, double d, double d2) {
    double d3 = std::cos(kcamera2->O1);
    double d4 = std::sin(kcamera2->O1);
    double d5 = std::cos(kcamera2->O2);
    double d6 = std::sin(kcamera2->O2);
    double d7 = std::cos(kcamera2->O3);
    double d8 = std::sin(kcamera2->O3);
    jint n = jint(0);
    while ((n < this->nbparticle))
    {
        double d9 = (this->posx[n] - kcamera2->X);
        double d10 = (this->posy[n] - kcamera2->Y);
        double d11 = (this->posz[n] - kcamera2->Z);
        double d12 = (((-d8) * d11) + (d7 * d9));
        double d13 = d10;
        double d14 = ((d7 * d11) + (d8 * d9));
        double d15 = d12;
        double d16 = ((d5 * d13) + (d6 * d14));
        double d17 = (((-d6) * d13) + (d5 * d14));
        (d9 = ((d3 * d15) + (d4 * d16)));
        (d10 = (((-d4) * d15) + (d3 * d16)));
        (d11 = d17);
        owner->rcs->Setzoom3D(d9, d10, d11, d, d2, kcamera2->FOV, kRenderContext2, kTexture2, 256.0);
        (++n);
    }
}

// kparticle.move — reconstructed Java line 537
void kparticle::move() {
    double d = 0.0;
    double d2 = 0.0;
    double d3 = 0.0;
    double d4 = (1.0 / double(this->nbparticle));
    jint n = jint(0);
    while ((n < this->nbparticle))
    {
        jint n2 = n;
        (this->Vx[n2] = (this->Vx[n2] + this->Ax[n]));
        jint n3 = n;
        (this->Vy[n3] = (this->Vy[n3] + this->Ay[n]));
        jint n4 = n;
        (this->Vz[n4] = (this->Vz[n4] + this->Az[n]));
        jint n5 = n;
        (this->posx[n5] = (this->posx[n5] + this->Vx[n]));
        jint n6 = n;
        (this->posy[n6] = (this->posy[n6] + this->Vy[n]));
        jint n7 = n;
        (this->posz[n7] = (this->posz[n7] + this->Vz[n]));
        (d += this->posx[n]);
        (d2 += this->posy[n]);
        (d3 += this->posz[n]);
        (++n);
    }
    (this->aveX = (d * d4));
    (this->aveY = (d2 * d4));
    (this->aveZ = (d3 * d4));
}

// kparticle.mvtfloor — reconstructed Java line 566
void kparticle::mvtfloor(double d, double d2, double d3, double d4) {
    double d5 = this->aveX;
    double d6 = this->aveY;
    double d7 = this->aveZ;
    jint n = jint(0);
    while ((n < this->nbparticle))
    {
        double d8 = this->posx[n];
        double d9 = this->posy[n];
        double d10 = this->posz[n];
        if ((d9 < d))
        {
            (this->posy[n] = d);
            jint n2 = n;
            (this->Vy[n2] = (this->Vy[n2] * (-1.0)));
        }
        if ((d9 > d2))
        {
            (this->posy[n] = d2);
            jint n3 = n;
            (this->Vy[n3] = (this->Vy[n3] * (-1.0)));
        }
        jint n4 = n;
        (this->Vx[n4] = (this->Vx[n4] * d4));
        jint n5 = n;
        (this->Vy[n5] = (this->Vy[n5] * d4));
        jint n6 = n;
        (this->Vz[n6] = (this->Vz[n6] * d4));
        (this->Ax[n] = 0.0);
        (this->Ay[n] = d3);
        (this->Az[n] = 0.0);
        (++n);
    }
    this->move();
}

// kparticle.mvtgalax — reconstructed Java line 599
void kparticle::mvtgalax() {
    double d = this->aveX;
    double d2 = this->aveY;
    double d3 = this->aveZ;
    jint n = jint(0);
    while ((n < this->nbparticle))
    {
        double d4 = this->posx[n];
        double d5 = this->posy[n];
        double d6 = this->posz[n];
        double d7 = (std::pow((((d4 * d4) + (d5 * d5)) + (d6 * d6)), 1.5) * 0.001);
        (this->Ax[n] = ((-d4) * d7));
        (this->Ay[n] = ((-d5) * d7));
        (this->Az[n] = ((-d6) * d7));
        (++n);
    }
    this->move();
}

// kparticle.reset — reconstructed Java line 617
void kparticle::reset(double d, double d2, double d3, double d4, double d5, double d6, double d7, double d8) {
    double d9 = 0.0;
    double d10 = 0.0;
    double d11 = 0.0;
    double d12 = (1.0 / double(this->nbparticle));
    double d13 = 0.0;
    double d14 = 0.0;
    double d15 = (((3.141592653589793 * jint(2)) * d) * d12);
    double d16 = (((3.141592653589793 * jint(2)) * d2) * d12);
    jint n = jint(0);
    while ((n < this->nbparticle))
    {
        double d17 = (d3 + (owner->random.random() * (d4 - d3)));
        double d18 = 0.0;
        double d19 = (std::cos(d13) * d17);
        double d20 = (std::sin(d13) * d17);
        (this->posx[n] = ((std::cos(d14) * d18) - (std::sin(d14) * d20)));
        (this->posy[n] = d19);
        (this->posz[n] = ((std::sin(d14) * d18) + (std::cos(d14) * d20)));
        (this->force[n] = 1.0);
        (d17 = 0.05);
        (this->Vx[n] = (this->posz[n] * d17));
        (this->Vy[n] = (this->posx[n] * d17));
        (this->Vz[n] = (this->posy[n] * d17));
        (d9 += this->posx[n]);
        (d10 += this->posy[n]);
        (d11 += this->posz[n]);
        (d13 += d15);
        (d14 += d16);
        (++n);
    }
    (this->aveX = (d9 * d12));
    (this->aveY = (d10 * d12));
    (this->aveZ = (d11 * d12));
}

// kTable.kTable — reconstructed Java line 658
kTable::kTable(kraycasting* owner, String string) : EngineObject(owner) {
    String string2{};
    (this->lengthtable = (owner->FindHowMany(string, String("|")) - jint(1)));
    (this->datetable = Array<double>(this->lengthtable));
    (this->floattable = Array<double>(this->lengthtable));
    jint n = jint(0);
    while ((n < this->lengthtable))
    {
        (string2 = owner->FindInto(string, (jint(1) + n), String("|"))->trim());
        (this->floattable[n] = jdouble(owner->FindInto(string2, jint(1), String(","))->trim()));
        (this->datetable[n] = jdouble(owner->FindInto(string2, jint(0), String(","))->trim()));
        (++n);
    }
}

// kTable.spline — reconstructed Java line 672
double kTable::spline(double d) {
    jint n = (this->lengthtable - jint(1));
    if ((d <= this->datetable[jint(0)]))
    {
        return this->floattable[jint(0)];
    }
    jint n2 = jint(0);
    jint n3 = n;
    jint n4 = ((n2 + n3) >> jint(1));
    jint n5 = n3;
    if ((d >= this->datetable[n5]))
    {
        return this->floattable[n5];
    }
    (n5 = n4);
    jint n6 = (n4 + jint(1));
    while (((!(d >= this->datetable[n5])) || (!(d < this->datetable[n6]))))
    {
        if ((d < this->datetable[n5]))
        {
            (n3 = n4);
        }
        else
        {
            (n2 = n4);
        }
        (n5 = (n4 = ((n2 + n3) >> jint(1))));
        (n6 = (n4 + jint(1)));
    }
    jint n7 = ((n4 == jint(0)) ? n5 : (n5 - jint(1)));
    jint n8 = (((n4 + jint(1)) == n) ? n6 : (n6 + jint(1)));
    double d2 = this->datetable[n5];
    double d3 = this->datetable[n6];
    double d4 = (d3 - d2);
    double d5 = ((this->floattable[n6] - this->floattable[n7]) / (d3 - this->datetable[n7]));
    double d6 = ((this->floattable[n8] - this->floattable[n5]) / (this->datetable[n8] - d2));
    double d7 = d5;
    double d8 = this->floattable[n5];
    double d9 = this->floattable[n6];
    double d10 = d4;
    double d11 = (d10 * d10);
    double d12 = ((((-(d10 * d6)) - ((d7 * 2.0) * d10)) + (3.0 * (d9 - d8))) / d11);
    double d13 = (((d6 - ((2.0 * d12) * d10)) - d7) / (3.0 * d11));
    (d11 = (d - d2));
    (d9 = (d11 * d11));
    double d14 = (d9 * d11);
    (d10 = ((((d13 * d14) + (d12 * d9)) + (d7 * d11)) + d8));
    return d10;
}

// kTimer.getTime — reconstructed Java line 720
jint kTimer::getTime() {
    return jint(((owner->platform.currentTimeMillis() - this->StartTime) / int64_t(10)));
}

// kTimer.reset — reconstructed Java line 724
void kTimer::reset() {
    (this->StartTime = owner->platform.currentTimeMillis());
}

// kParameterX.kParameterX — reconstructed Java line 767
kParameterX::kParameterX(kraycasting* owner) : EngineObject(owner) {
}

// kParameterXCTE.kParameterXCTE — reconstructed Java line 777
kParameterXCTE::kParameterXCTE(kraycasting* owner) : kParameterX(owner) {
}

// kParameterXCTE.Out — reconstructed Java line 780
Ref<Object> kParameterXCTE::Out() {
    return nullptr;
}

// kParameterXCTE.doubleout — reconstructed Java line 784
double kParameterXCTE::doubleout(double d) {
    return this->a;
}

// kParameterXAFF.kParameterXAFF — reconstructed Java line 791
kParameterXAFF::kParameterXAFF(kraycasting* owner) : kParameterX(owner) {
}

// kParameterXAFF.Out — reconstructed Java line 794
Ref<Object> kParameterXAFF::Out() {
    return nullptr;
}

// kParameterXAFF.doubleout — reconstructed Java line 798
double kParameterXAFF::doubleout(double d) {
    return (this->a + (this->b * d));
}

// kParameterXRND.kParameterXRND — reconstructed Java line 805
kParameterXRND::kParameterXRND(kraycasting* owner) : kParameterX(owner) {
}

// kParameterXRND.Out — reconstructed Java line 808
Ref<Object> kParameterXRND::Out() {
    return nullptr;
}

// kParameterXRND.doubleout — reconstructed Java line 812
double kParameterXRND::doubleout(double d) {
    return (this->a + (owner->random.random() * (this->b - this->a)));
}

// kParameterXSPLINE.kParameterXSPLINE — reconstructed Java line 819
kParameterXSPLINE::kParameterXSPLINE(kraycasting* owner) : kParameterX(owner) {
}

// kParameterXSPLINE.Out — reconstructed Java line 822
Ref<Object> kParameterXSPLINE::Out() {
    return nullptr;
}

// kParameterXSPLINE.doubleout — reconstructed Java line 826
double kParameterXSPLINE::doubleout(double d) {
    return this->spltable->spline((d - this->a));
}

// kParameterXSIN.kParameterXSIN — reconstructed Java line 833
kParameterXSIN::kParameterXSIN(kraycasting* owner) : kParameterX(owner) {
}

// kParameterXSIN.Out — reconstructed Java line 836
Ref<Object> kParameterXSIN::Out() {
    return nullptr;
}

// kParameterXSIN.doubleout — reconstructed Java line 840
double kParameterXSIN::doubleout(double d) {
    return (this->a + (std::sin((this->b * d)) * this->c));
}

// kParameterXCOS.kParameterXCOS — reconstructed Java line 847
kParameterXCOS::kParameterXCOS(kraycasting* owner) : kParameterX(owner) {
}

// kParameterXCOS.Out — reconstructed Java line 850
Ref<Object> kParameterXCOS::Out() {
    return nullptr;
}

// kParameterXCOS.doubleout — reconstructed Java line 854
double kParameterXCOS::doubleout(double d) {
    return (this->a + (std::cos((this->b * d)) * this->c));
}

// kParameterXOB.kParameterXOB — reconstructed Java line 861
kParameterXOB::kParameterXOB(kraycasting* owner) : kParameterX(owner) {
}

// kParameterXOB.Out — reconstructed Java line 864
Ref<Object> kParameterXOB::Out() {
    if ((this->kOBJ == nullptr))
    {
        return owner->RenderContext;
    }
    return this->kOBJ;
}

// kParameterXOB.doubleout — reconstructed Java line 871
double kParameterXOB::doubleout(double d) {
    return 0.0;
}

// kEffectX.kEffectX — reconstructed Java line 882
kEffectX::kEffectX(kraycasting* owner) : EngineObject(owner) {
    (this->hhf = std::make_shared<helpfade>(owner));
}

// kEffectX.rendah — reconstructed Java line 886
void kEffectX::rendah(jint n) {
    double d{};
    double d2 = jdouble(n);
    if ((this->keffectcode == jint(1)))
    {
        [&]() { auto argument0 = this->kParameterXEffectX[jint(0)]->doubleout(d2); auto argument1 = this->kParameterXEffectX[jint(1)]->doubleout(d2); auto argument2 = 0.0; auto argument3 = 0.0; auto argument4 = 0.0; auto argument5 = 0.0; auto argument6 = 0.0; auto argument7 = std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(9)]->Out()); return owner->rcs->Position3D(argument0, argument1, argument2, argument3, argument4, argument5, argument6, argument7); }();
        [&]() { auto argument0 = this->kParameterXEffectX[jint(2)]->doubleout(d2); auto argument1 = this->kParameterXEffectX[jint(3)]->doubleout(d2); auto argument2 = this->kParameterXEffectX[jint(4)]->doubleout(d2); auto argument3 = this->kParameterXEffectX[jint(5)]->doubleout(d2); auto argument4 = this->kParameterXEffectX[jint(6)]->doubleout(d2); auto argument5 = this->kParameterXEffectX[jint(7)]->doubleout(d2); auto argument6 = std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(9)]->Out()); return owner->rcs->findtwirl(argument0, argument1, argument2, argument3, argument4, argument5, argument6); }();
        owner->rcs->draw(std::static_pointer_cast<kTexture>(this->kParameterXEffectX[jint(8)]->Out()), std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(9)]->Out()));
    }
    if ((this->keffectcode == jint(2)))
    {
        [&]() { auto argument0 = this->kParameterXEffectX[jint(1)]->doubleout(d2); auto argument1 = this->kParameterXEffectX[jint(2)]->doubleout(d2); auto argument2 = this->kParameterXEffectX[jint(3)]->doubleout(d2); auto argument3 = this->kParameterXEffectX[jint(4)]->doubleout(d2); auto argument4 = std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(5)]->Out()); auto argument5 = std::static_pointer_cast<kTexture>(this->kParameterXEffectX[jint(0)]->Out()); auto argument6 = this->kParameterXEffectX[jint(6)]->doubleout(d2); return owner->rcs->SetzoomP(argument0, argument1, argument2, argument3, argument4, argument5, argument6); }();
    }
    if ((this->keffectcode == jint(3)))
    {
        (this->hhf->Frd1 = this->kParameterXEffectX[jint(5)]->doubleout(d2));
        (this->hhf->Frv1 = this->kParameterXEffectX[jint(6)]->doubleout(d2));
        (this->hhf->Frd2 = this->kParameterXEffectX[jint(7)]->doubleout(d2));
        (this->hhf->Frv2 = this->kParameterXEffectX[jint(8)]->doubleout(d2));
        (this->hhf->Fgd1 = this->kParameterXEffectX[jint(9)]->doubleout(d2));
        (this->hhf->Fgv1 = this->kParameterXEffectX[jint(10)]->doubleout(d2));
        (this->hhf->Fgd2 = this->kParameterXEffectX[jint(11)]->doubleout(d2));
        (this->hhf->Fgv2 = this->kParameterXEffectX[jint(12)]->doubleout(d2));
        (this->hhf->Fbd1 = this->kParameterXEffectX[jint(13)]->doubleout(d2));
        (this->hhf->Fbv1 = this->kParameterXEffectX[jint(14)]->doubleout(d2));
        (this->hhf->Fbd2 = this->kParameterXEffectX[jint(15)]->doubleout(d2));
        (this->hhf->Fbv2 = this->kParameterXEffectX[jint(16)]->doubleout(d2));
        owner->rcs->Position3D(0.0, 0.0, std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->Z, std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->O1, std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->O2, std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->O3, std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->FOV, std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(4)]->Out()));
        [&]() { auto argument0 = this->kParameterXEffectX[jint(1)]->doubleout(d2); auto argument1 = this->kParameterXEffectX[jint(2)]->doubleout(d2); auto argument2 = std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(4)]->Out()); auto argument3 = this->hhf; return owner->rcs->findtunnel(argument0, argument1, argument2, argument3); }();
        owner->rcs->draw(std::static_pointer_cast<kTexture>(this->kParameterXEffectX[jint(3)]->Out()), std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(4)]->Out()));
    }
    if ((this->keffectcode == jint(4)))
    {
        (this->hhf->Frd1 = this->kParameterXEffectX[jint(7)]->doubleout(d2));
        (this->hhf->Frv1 = this->kParameterXEffectX[jint(8)]->doubleout(d2));
        (this->hhf->Frd2 = this->kParameterXEffectX[jint(9)]->doubleout(d2));
        (this->hhf->Frv2 = this->kParameterXEffectX[jint(10)]->doubleout(d2));
        (this->hhf->Fgd1 = this->kParameterXEffectX[jint(11)]->doubleout(d2));
        (this->hhf->Fgv1 = this->kParameterXEffectX[jint(12)]->doubleout(d2));
        (this->hhf->Fgd2 = this->kParameterXEffectX[jint(13)]->doubleout(d2));
        (this->hhf->Fgv2 = this->kParameterXEffectX[jint(14)]->doubleout(d2));
        (this->hhf->Fbd1 = this->kParameterXEffectX[jint(15)]->doubleout(d2));
        (this->hhf->Fbv1 = this->kParameterXEffectX[jint(16)]->doubleout(d2));
        (this->hhf->Fbd2 = this->kParameterXEffectX[jint(17)]->doubleout(d2));
        (this->hhf->Fbv2 = this->kParameterXEffectX[jint(18)]->doubleout(d2));
        owner->rcs->Position3D(std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->X, std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->Y, std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->Z, std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->O1, std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->O2, std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->O3, std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->FOV, std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(4)]->Out()));
        [&]() { auto argument0 = this->kParameterXEffectX[jint(1)]->doubleout(d2); auto argument1 = this->kParameterXEffectX[jint(2)]->doubleout(d2); auto argument2 = this->kParameterXEffectX[jint(5)]->doubleout(d2); auto argument3 = this->kParameterXEffectX[jint(6)]->doubleout(d2); auto argument4 = std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(4)]->Out()); auto argument5 = this->hhf; return owner->rcs->findfloor(argument0, argument1, argument2, argument3, argument4, argument5); }();
        owner->rcs->draw(std::static_pointer_cast<kTexture>(this->kParameterXEffectX[jint(3)]->Out()), std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(4)]->Out()));
    }
    if ((this->keffectcode == jint(5)))
    {
        [&]() { auto argument0 = this->kParameterXEffectX[jint(0)]->doubleout(d2); auto argument1 = this->kParameterXEffectX[jint(1)]->doubleout(d2); auto argument2 = this->kParameterXEffectX[jint(2)]->doubleout(d2); auto argument3 = this->kParameterXEffectX[jint(3)]->doubleout(d2); auto argument4 = this->kParameterXEffectX[jint(4)]->doubleout(d2); auto argument5 = this->kParameterXEffectX[jint(5)]->doubleout(d2); auto argument6 = std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(7)]->Out()); auto argument7 = std::static_pointer_cast<kTexture>(this->kParameterXEffectX[jint(6)]->Out()); auto argument8 = this->kParameterXEffectX[jint(8)]->doubleout(d2); return owner->rcs->Setzoom3D(argument0, argument1, argument2, argument3, argument4, argument5, argument6, argument7, argument8); }();
    }
    if ((this->keffectcode == jint(6)))
    {
        [&]() { auto argument0 = owner->RenderContext; auto argument1 = this->kParameterXEffectX[jint(1)]->doubleout(d2); auto argument2 = this->kParameterXEffectX[jint(2)]->doubleout(d2); auto argument3 = this->kParameterXEffectX[jint(3)]->doubleout(d2); auto argument4 = this->kParameterXEffectX[jint(4)]->doubleout(d2); return std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(0)]->Out())->SetRect(argument0, argument1, argument2, argument3, argument4); }();
    }
    if ((this->keffectcode == jint(7)))
    {
        [&]() { auto argument0 = this->kParameterXEffectX[jint(0)]->doubleout(d2); auto argument1 = this->kParameterXEffectX[jint(1)]->doubleout(d2); auto argument2 = this->kParameterXEffectX[jint(2)]->doubleout(d2); auto argument3 = this->kParameterXEffectX[jint(3)]->doubleout(d2); auto argument4 = this->kParameterXEffectX[jint(4)]->doubleout(d2); auto argument5 = this->kParameterXEffectX[jint(5)]->doubleout(d2); auto argument6 = std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(7)]->Out()); auto argument7 = std::static_pointer_cast<kTexture>(this->kParameterXEffectX[jint(6)]->Out()); return owner->rcs->flare3D(argument0, argument1, argument2, argument3, argument4, argument5, argument6, argument7); }();
    }
    if ((this->keffectcode == jint(8)))
    {
        owner->rcs->MotionBlur(std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(0)]->Out()));
    }
    if ((this->keffectcode == jint(9)))
    {
        double d3 = this->kParameterXEffectX[jint(4)]->doubleout(d2);
        if ((d3 == 0.0))
        {
            std::static_pointer_cast<kparticle>(this->kParameterXEffectX[jint(0)]->Out())->mvtgalax();
        }
        else
        if ((d3 == 1.0))
        {
            [&]() { auto argument0 = this->kParameterXEffectX[jint(9)]->doubleout(d2); auto argument1 = this->kParameterXEffectX[jint(10)]->doubleout(d2); auto argument2 = this->kParameterXEffectX[jint(11)]->doubleout(d2); auto argument3 = this->kParameterXEffectX[jint(12)]->doubleout(d2); return std::static_pointer_cast<kparticle>(this->kParameterXEffectX[jint(0)]->Out())->mvtfloor(argument0, argument1, argument2, argument3); }();
        }
        (d = this->kParameterXEffectX[jint(5)]->doubleout(d2));
        if ((d == 0.0))
        {
            [&]() { auto argument0 = std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(6)]->Out()); auto argument1 = std::static_pointer_cast<kTexture>(this->kParameterXEffectX[jint(2)]->Out()); auto argument2 = std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(1)]->Out()); auto argument3 = this->kParameterXEffectX[jint(7)]->doubleout(d2); auto argument4 = this->kParameterXEffectX[jint(8)]->doubleout(d2); return std::static_pointer_cast<kparticle>(this->kParameterXEffectX[jint(0)]->Out())->drawflare(argument0, argument1, argument2, argument3, argument4); }();
        }
        if ((d == 2.0))
        {
            [&]() { auto argument0 = std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(6)]->Out()); auto argument1 = std::static_pointer_cast<kTexture>(this->kParameterXEffectX[jint(2)]->Out()); auto argument2 = std::static_pointer_cast<kRenderContext>(this->kParameterXEffectX[jint(1)]->Out()); auto argument3 = this->kParameterXEffectX[jint(7)]->doubleout(d2); auto argument4 = this->kParameterXEffectX[jint(8)]->doubleout(d2); return std::static_pointer_cast<kparticle>(this->kParameterXEffectX[jint(0)]->Out())->drawsprit(argument0, argument1, argument2, argument3, argument4); }();
        }
    }
    if ((this->keffectcode == jint(10)))
    {
        [&]() { auto argument0 = this->kParameterXEffectX[jint(1)]->doubleout(d2); auto argument1 = this->kParameterXEffectX[jint(2)]->doubleout(d2); auto argument2 = this->kParameterXEffectX[jint(3)]->doubleout(d2); auto argument3 = this->kParameterXEffectX[jint(4)]->doubleout(d2); auto argument4 = this->kParameterXEffectX[jint(5)]->doubleout(d2); auto argument5 = this->kParameterXEffectX[jint(6)]->doubleout(d2); auto argument6 = this->kParameterXEffectX[jint(7)]->doubleout(d2); return std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->set(argument0, argument1, argument2, argument3, argument4, argument5, argument6); }();
    }
    if ((this->keffectcode == jint(11)))
    {
        [&]() { auto argument0 = this->kParameterXEffectX[jint(1)]->doubleout(d2); auto argument1 = this->kParameterXEffectX[jint(2)]->doubleout(d2); auto argument2 = this->kParameterXEffectX[jint(3)]->doubleout(d2); auto argument3 = this->kParameterXEffectX[jint(4)]->doubleout(d2); auto argument4 = this->kParameterXEffectX[jint(5)]->doubleout(d2); auto argument5 = this->kParameterXEffectX[jint(6)]->doubleout(d2); auto argument6 = this->kParameterXEffectX[jint(7)]->doubleout(d2); auto argument7 = this->kParameterXEffectX[jint(8)]->doubleout(d2); return std::static_pointer_cast<kcamera>(this->kParameterXEffectX[jint(0)]->Out())->target(argument0, argument1, argument2, argument3, argument4, argument5, argument6, argument7); }();
    }
    if ((this->keffectcode == jint(12)))
    {
        (d = this->kParameterXEffectX[jint(1)]->doubleout(d2));
        if ((d2 < d))
        {
            (std::static_pointer_cast<kSound>(this->kParameterXEffectX[jint(0)]->Out())->playedonce = false);
        }
        else
        if ((!std::static_pointer_cast<kSound>(this->kParameterXEffectX[jint(0)]->Out())->playedonce))
        {
            std::static_pointer_cast<kSound>(this->kParameterXEffectX[jint(0)]->Out())->PlayOne();
        }
    }
}

// kSoundCommand.kSoundCommand — reconstructed Java line 979
kSoundCommand::kSoundCommand(kraycasting* owner) : EngineObject(owner) {
}

// kSoundCommand.MakeSoundCommand — reconstructed Java line 982
void kSoundCommand::MakeSoundCommand() {
    if ((this->commandcode == jint(0)))
    {
        this->snd->StopOne();
    }
    if ((this->commandcode == jint(1)))
    {
        this->snd->PlayOne();
    }
    if ((this->commandcode == jint(2)))
    {
        this->snd->LoopOne();
    }
}

// kPartX.kPartX — reconstructed Java line 1003
kPartX::kPartX(kraycasting* owner) : EngineObject(owner) {
}

// kPartX.rendah — reconstructed Java line 1006
void kPartX::rendah(jint n) {
    jint n2 = ((n - this->TimeLength) + this->TimeStart);
    jint n3 = jint(0);
    while ((n3 < this->NBEffect))
    {
        this->kEffectXPartX[n3]->rendah(n2);
        (++n3);
    }
    if ((((owner)->THEkScript->lastpartplayed != this) && (this->SoundCommands != nullptr)))
    {
        (n3 = jint(0));
        while ((n3 < this->NBSoundCommands))
        {
            this->SoundCommands[n3]->MakeSoundCommand();
            (++n3);
        }
    }
    ((owner)->THEkScript->lastpartplayed = raw(this));
}

// kScript.kScript — reconstructed Java line 1031
kScript::kScript(kraycasting* owner, String string) : EngineObject(owner) {
    jint n{};
    jint n2{};
    Array<String> stringArray = owner->SplitSTag(string, String("KIMAGES"));
    Array<String> stringArray2 = owner->SplitSTag(string, String("KSOUNDS"));
    Array<String> stringArray3 = owner->SplitSTag(string, String("KRECT"));
    Array<String> stringArray4 = owner->SplitSTag(string, String("KTABLE"));
    Array<String> stringArray5 = owner->SplitSTag(string, String("KSCRIPT"));
    Array<String> stringArray6 = owner->SplitSTag(string, String("KTICLE"));
    Array<String> stringArray7 = owner->SplitSTag(string, String("KCAM"));
    (owner->TableHash = Hashtable());
    jint n3 = jint(0);
    while ((n3 < stringArray4->length))
    {
        Hashtable hashtable = owner->TableHash;
        String string2 = owner->FindInto(stringArray4[n3], jint(0), String("|"))->trim();
        kraycasting* kraycasting3 = owner;
        hashtable->put(string2, std::make_shared<kTable>(owner, stringArray4[n3]->trim()));
        (++n3);
    }
    (owner->ImageHash = Hashtable());
    (n3 = jint(0));
    while ((n3 < stringArray->length))
    {
        Hashtable hashtable = owner->ImageHash;
        String string3 = owner->FindInto(stringArray[n3], jint(0), String("|"))->trim();
        kraycasting* kraycasting4 = owner;
        hashtable->put(string3, std::make_shared<kTexture>(owner, owner->getImage(owner->getDocumentBase(), owner->FindInto(stringArray[n3], jint(1), String("|"))->trim())));
        (++n3);
    }
    (owner->partcHash = Hashtable());
    (n3 = jint(0));
    while ((n3 < stringArray6->length))
    {
        Hashtable hashtable = owner->partcHash;
        String string4 = owner->FindInto(stringArray6[n3], jint(0), String("|"))->trim();
        kraycasting* kraycasting5 = owner;
        hashtable->put(string4, std::make_shared<kparticle>(owner, jinteger(owner->FindInto(stringArray6[n3], jint(1), String("|"))->trim()), jdouble(owner->FindInto(stringArray6[n3], jint(2), String("|"))->trim()), jdouble(owner->FindInto(stringArray6[n3], jint(3), String("|"))->trim()), jdouble(owner->FindInto(stringArray6[n3], jint(4), String("|"))->trim()), jdouble(owner->FindInto(stringArray6[n3], jint(5), String("|"))->trim()), jdouble(owner->FindInto(stringArray6[n3], jint(6), String("|"))->trim()), jdouble(owner->FindInto(stringArray6[n3], jint(7), String("|"))->trim()), jdouble(owner->FindInto(stringArray6[n3], jint(8), String("|"))->trim()), jdouble(owner->FindInto(stringArray6[n3], jint(9), String("|"))->trim())));
        (++n3);
    }
    (owner->CamerHash = Hashtable());
    (n3 = jint(0));
    while ((n3 < stringArray7->length))
    {
        Hashtable hashtable = owner->CamerHash;
        String string5 = owner->FindInto(stringArray7[n3], jint(0), String("|"))->trim();
        kraycasting* kraycasting6 = owner;
        hashtable->put(string5, std::make_shared<kcamera>(owner, jinteger(owner->FindInto(stringArray7[n3], jint(1), String("|"))->trim())->intValue(), jinteger(owner->FindInto(stringArray7[n3], jint(2), String("|"))->trim())->intValue(), jinteger(owner->FindInto(stringArray7[n3], jint(3), String("|"))->trim())->intValue(), jinteger(owner->FindInto(stringArray7[n3], jint(4), String("|"))->trim())->intValue(), jinteger(owner->FindInto(stringArray7[n3], jint(5), String("|"))->trim())->intValue(), jinteger(owner->FindInto(stringArray7[n3], jint(6), String("|"))->trim())->intValue(), jinteger(owner->FindInto(stringArray7[n3], jint(7), String("|"))->trim())->intValue()));
        (++n3);
    }
    (owner->RectaHash = Hashtable());
    (n3 = jint(0));
    while ((n3 < stringArray3->length))
    {
        Hashtable hashtable = owner->RectaHash;
        String string6 = owner->FindInto(stringArray3[n3], jint(0), String("|"))->trim();
        kraycasting* kraycasting7 = owner;
        hashtable->put(string6, std::make_shared<kRenderContext>(owner, owner->RenderContext, jdouble(owner->FindInto(stringArray3[n3], jint(1), String("|"))->trim()), jdouble(owner->FindInto(stringArray3[n3], jint(2), String("|"))->trim()), jdouble(owner->FindInto(stringArray3[n3], jint(3), String("|"))->trim()), jdouble(owner->FindInto(stringArray3[n3], jint(4), String("|"))->trim())));
        (++n3);
    }
    (owner->SoundHash = Hashtable());
    (n3 = jint(0));
    while ((n3 < stringArray2->length))
    {
        owner->SoundHash->put(owner->FindInto(stringArray2[n3], jint(0), String("|"))->trim(), std::make_shared<kSound>(owner, owner->getAudioClip(owner->getDocumentBase(), owner->FindInto(stringArray2[n3], jint(1), String("|"))->trim())));
        (++n3);
    }
    Array<String> stringArray8 = owner->SplitSTag(stringArray5[jint(0)], String("KPART"));
    (this->NBPart = stringArray8->length);
    (this->kPartXscript = Array<Ref<kPartX>>((this->NBPart + jint(1))));
    Array<String> stringArray9 = owner->SplitSTag(stringArray5[jint(0)], String("KEND"));
    (this->EndPref = jint(0));
    if ((stringArray9[jint(0)]->trim()->toLowerCase()->compareTo(String("loop")) == jint(0)))
    {
        (this->EndPref = jint(1));
    }
    jint n4 = jint(0);
    while ((n4 < stringArray8->length))
    {
        String string7{};
        (stringArray9 = owner->SplitSTag(stringArray8[n4], String("D")));
        Array<String> stringArray10 = owner->SplitSTag(stringArray8[n4], String("S"));
        (this->kPartXscript[n4] = std::make_shared<kPartX>(owner));
        if ((stringArray10 != nullptr))
        {
            (this->kPartXscript[n4]->NBSoundCommands = stringArray10->length);
            (this->kPartXscript[n4]->SoundCommands = Array<Ref<kSoundCommand>>(this->kPartXscript[n4]->NBSoundCommands));
            (n2 = jint(0));
            while ((n2 < this->kPartXscript[n4]->NBSoundCommands))
            {
                (n = jint(0));
                (string7 = owner->FindInto(stringArray10[n2], jint(1), String(","))->trim()->toLowerCase());
                if ((string7->compareTo(String("play")) == jint(0)))
                {
                    (n = jint(1));
                }
                if ((string7->compareTo(String("loop")) == jint(0)))
                {
                    (n = jint(2));
                }
                (this->kPartXscript[n4]->SoundCommands[n2] = std::make_shared<kSoundCommand>(owner));
                (this->kPartXscript[n4]->SoundCommands[n2]->commandcode = n);
                (this->kPartXscript[n4]->SoundCommands[n2]->snd = std::static_pointer_cast<kSound>(owner->SoundHash->get(owner->FindInto(stringArray10[n2], jint(0), String(",")))));
                (++n2);
            }
        }
        (this->kPartXscript[n4]->TimeLength = jinteger(owner->FindInto(stringArray9[jint(0)], jint(0), String(","))->trim()));
        (this->kPartXscript[n4]->TimeStart = jinteger(owner->FindInto(stringArray9[jint(0)], jint(1), String(","))->trim()));
        Array<String> stringArray11 = owner->SplitSTag(stringArray8[n4], String("Fx"));
        (this->kPartXscript[n4]->NBEffect = stringArray11->length);
        (this->kPartXscript[n4]->kEffectXPartX = Array<Ref<kEffectX>>(this->kPartXscript[n4]->NBEffect));
        (n2 = jint(0));
        while ((n2 < stringArray11->length))
        {
            (this->kPartXscript[n4]->kEffectXPartX[n2] = std::make_shared<kEffectX>(owner));
            Array<String> stringArray12 = owner->SplitSTag(stringArray11[n2], String("Pa"));
            (this->kPartXscript[n4]->kEffectXPartX[n2]->knbparam = stringArray12->length);
            (this->kPartXscript[n4]->kEffectXPartX[n2]->kParameterXEffectX = Array<Ref<kParameterX>>(this->kPartXscript[n4]->kEffectXPartX[n2]->knbparam));
            (string7 = owner->FindInto(stringArray12[jint(0)], jint(0), String("|"))->trim()->toLowerCase());
            (n = jint(0));
            if ((string7->compareTo(String("twirl")) == jint(0)))
            {
                (n = jint(1));
            }
            if ((string7->compareTo(String("sprit")) == jint(0)))
            {
                (n = jint(2));
            }
            if ((string7->compareTo(String("tunnel")) == jint(0)))
            {
                (n = jint(3));
            }
            if ((string7->compareTo(String("ground")) == jint(0)))
            {
                (n = jint(4));
            }
            if ((string7->compareTo(String("sprit3d")) == jint(0)))
            {
                (n = jint(5));
            }
            if ((string7->compareTo(String("setrect")) == jint(0)))
            {
                (n = jint(6));
            }
            if ((string7->compareTo(String("flare3d")) == jint(0)))
            {
                (n = jint(7));
            }
            if ((string7->compareTo(String("motionblur")) == jint(0)))
            {
                (n = jint(8));
            }
            if ((string7->compareTo(String("particle")) == jint(0)))
            {
                (n = jint(9));
            }
            if ((string7->compareTo(String("setcamcoord")) == jint(0)))
            {
                (n = jint(10));
            }
            if ((string7->compareTo(String("setcamtarget")) == jint(0)))
            {
                (n = jint(11));
            }
            if ((string7->compareTo(String("evesound")) == jint(0)))
            {
                (n = jint(12));
            }
            (this->kPartXscript[n4]->kEffectXPartX[n2]->keffectcode = n);
            (n3 = jint(1));
            while ((n3 < this->kPartXscript[n4]->kEffectXPartX[n2]->knbparam))
            {
                Ref<kParameterX> kParameterX2{};
                (string7 = owner->FindInto(stringArray12[n3], jint(0), String("|"))->trim()->toLowerCase());
                if ((string7->compareTo(String("ima")) == jint(0)))
                {
                    (kParameterX2 = std::make_shared<kParameterXOB>(owner));
                    (kParameterX2->CodeParam = jint(1));
                    (kParameterX2->kOBJ = std::static_pointer_cast<kTexture>(owner->ImageHash->get(owner->FindInto(stringArray12[n3], jint(1), String("|")))));
                    (std::static_pointer_cast<kEffectX>(this->kPartXscript[n4]->kEffectXPartX[n2])->kParameterXEffectX[(n3 - jint(1))] = kParameterX2);
                }
                if ((string7->compareTo(String("cos")) == jint(0)))
                {
                    (kParameterX2 = std::make_shared<kParameterXCOS>(owner));
                    (kParameterX2->CodeParam = jint(2));
                    (kParameterX2->a = jdouble(owner->FindInto(stringArray12[n3], jint(1), String("|"))));
                    (kParameterX2->b = jdouble(owner->FindInto(stringArray12[n3], jint(2), String("|"))));
                    (kParameterX2->c = jdouble(owner->FindInto(stringArray12[n3], jint(3), String("|"))));
                    (std::static_pointer_cast<kEffectX>(this->kPartXscript[n4]->kEffectXPartX[n2])->kParameterXEffectX[(n3 - jint(1))] = kParameterX2);
                }
                if ((string7->compareTo(String("sin")) == jint(0)))
                {
                    (kParameterX2 = std::make_shared<kParameterXSIN>(owner));
                    (kParameterX2->CodeParam = jint(3));
                    (kParameterX2->a = jdouble(owner->FindInto(stringArray12[n3], jint(1), String("|"))));
                    (kParameterX2->b = jdouble(owner->FindInto(stringArray12[n3], jint(2), String("|"))));
                    (kParameterX2->c = jdouble(owner->FindInto(stringArray12[n3], jint(3), String("|"))));
                    (std::static_pointer_cast<kEffectX>(this->kPartXscript[n4]->kEffectXPartX[n2])->kParameterXEffectX[(n3 - jint(1))] = kParameterX2);
                }
                if ((string7->compareTo(String("cte")) == jint(0)))
                {
                    (kParameterX2 = std::make_shared<kParameterXCTE>(owner));
                    (kParameterX2->CodeParam = jint(4));
                    (kParameterX2->a = jdouble(owner->FindInto(stringArray12[n3], jint(1), String("|"))));
                    (std::static_pointer_cast<kEffectX>(this->kPartXscript[n4]->kEffectXPartX[n2])->kParameterXEffectX[(n3 - jint(1))] = kParameterX2);
                }
                if ((string7->compareTo(String("rnd")) == jint(0)))
                {
                    (kParameterX2 = std::make_shared<kParameterXRND>(owner));
                    (kParameterX2->CodeParam = jint(5));
                    (kParameterX2->a = jdouble(owner->FindInto(stringArray12[n3], jint(1), String("|"))));
                    (kParameterX2->b = jdouble(owner->FindInto(stringArray12[n3], jint(2), String("|"))));
                    (std::static_pointer_cast<kEffectX>(this->kPartXscript[n4]->kEffectXPartX[n2])->kParameterXEffectX[(n3 - jint(1))] = kParameterX2);
                }
                if ((string7->compareTo(String("spl")) == jint(0)))
                {
                    (kParameterX2 = std::make_shared<kParameterXSPLINE>(owner));
                    (kParameterX2->CodeParam = jint(6));
                    (kParameterX2->spltable = std::static_pointer_cast<kTable>(owner->TableHash->get(owner->FindInto(stringArray12[n3], jint(1), String("|")))));
                    (kParameterX2->a = jdouble(owner->FindInto(stringArray12[n3], jint(2), String("|"))));
                    (std::static_pointer_cast<kEffectX>(this->kPartXscript[n4]->kEffectXPartX[n2])->kParameterXEffectX[(n3 - jint(1))] = kParameterX2);
                }
                if ((string7->compareTo(String("aff")) == jint(0)))
                {
                    (kParameterX2 = std::make_shared<kParameterXAFF>(owner));
                    (kParameterX2->CodeParam = jint(7));
                    (kParameterX2->a = jdouble(owner->FindInto(stringArray12[n3], jint(1), String("|"))));
                    (kParameterX2->b = jdouble(owner->FindInto(stringArray12[n3], jint(2), String("|"))));
                    (std::static_pointer_cast<kEffectX>(this->kPartXscript[n4]->kEffectXPartX[n2])->kParameterXEffectX[(n3 - jint(1))] = kParameterX2);
                }
                if ((string7->compareTo(String("rec")) == jint(0)))
                {
                    (kParameterX2 = std::make_shared<kParameterXOB>(owner));
                    (kParameterX2->CodeParam = jint(8));
                    (kParameterX2->kOBJ = std::static_pointer_cast<kRenderContext>(owner->RectaHash->get(owner->FindInto(stringArray12[n3], jint(1), String("|")))));
                    (std::static_pointer_cast<kEffectX>(this->kPartXscript[n4]->kEffectXPartX[n2])->kParameterXEffectX[(n3 - jint(1))] = kParameterX2);
                }
                if ((string7->compareTo(String("par")) == jint(0)))
                {
                    (kParameterX2 = std::make_shared<kParameterXOB>(owner));
                    (kParameterX2->CodeParam = jint(9));
                    (kParameterX2->kOBJ = std::static_pointer_cast<kparticle>(owner->partcHash->get(owner->FindInto(stringArray12[n3], jint(1), String("|")))));
                    (std::static_pointer_cast<kEffectX>(this->kPartXscript[n4]->kEffectXPartX[n2])->kParameterXEffectX[(n3 - jint(1))] = kParameterX2);
                }
                if ((string7->compareTo(String("cam")) == jint(0)))
                {
                    (kParameterX2 = std::make_shared<kParameterXOB>(owner));
                    (kParameterX2->CodeParam = jint(10));
                    (kParameterX2->kOBJ = std::static_pointer_cast<kcamera>(owner->CamerHash->get(owner->FindInto(stringArray12[n3], jint(1), String("|")))));
                    (std::static_pointer_cast<kEffectX>(this->kPartXscript[n4]->kEffectXPartX[n2])->kParameterXEffectX[(n3 - jint(1))] = kParameterX2);
                }
                if ((string7->compareTo(String("snd")) == jint(0)))
                {
                    (kParameterX2 = std::make_shared<kParameterXOB>(owner));
                    (kParameterX2->CodeParam = jint(11));
                    (kParameterX2->kOBJ = std::static_pointer_cast<kSound>(owner->SoundHash->get(owner->FindInto(stringArray12[n3], jint(1), String("|")))));
                    (std::static_pointer_cast<kEffectX>(this->kPartXscript[n4]->kEffectXPartX[n2])->kParameterXEffectX[(n3 - jint(1))] = kParameterX2);
                }
                (++n3);
            }
            (++n2);
        }
        (++n4);
    }
    (this->kPartXscript[n4] = std::make_shared<kPartX>(owner));
    (n = jint(0));
    (n3 = jint(0));
    while ((n3 < this->NBPart))
    {
        (n2 = this->kPartXscript[n3]->TimeLength);
        (this->kPartXscript[n3]->TimeLength = n);
        (n += n2);
        (++n3);
    }
    (this->kPartXscript[n4]->TimeLength = n);
    (this->EndDate = n);
}

// kScript.part — reconstructed Java line 1284
Ref<kPartX> kScript::part(jint n) {
    jint n2 = jint(0);
    jint n3 = jint(0);
    while ((n3 < this->NBPart))
    {
        if (((this->kPartXscript[(n3 + jint(1))]->TimeLength >= n) && (this->kPartXscript[n3]->TimeLength < n)))
        {
            (n2 = n3);
        }
        (++n3);
    }
    if ((n >= this->EndDate))
    {
        if ((this->EndPref == jint(1)))
        {
            owner->kTime->reset();
        }
        else
        {
            (owner->boolend = true);
        }
    }
    return this->kPartXscript[n2];
}

// raycasting.raycasting — reconstructed Java line 1308
raycasting::raycasting(kraycasting* owner, Ref<kRenderContext> kRenderContext2) : EngineObject(owner) {
    (this->Rootrc = kRenderContext2);
    kraycasting* kraycasting3 = owner;
    (this->OneRec = std::make_shared<kRenderContext>(owner, kRenderContext2));
}

// raycasting.MotionBlur — reconstructed Java line 1315
void raycasting::MotionBlur(Ref<kRenderContext> kRenderContext2) {
    jint n = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    jint n2 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    jint n3 = kRenderContext2->Modulo;
    jint n4 = (kRenderContext2->ClipX1 + (kRenderContext2->ClipY1 * kRenderContext2->Modulo));
    Array<jint> nArray = std::static_pointer_cast<kRenderContext>(kRenderContext2)->RC_RootContext->CurrentChunk;
    Array<jint> nArray2 = std::static_pointer_cast<kRenderContext>(kRenderContext2)->RC_RootContext->Chunk2;
    if ((n == jint(0)))
    {
        return;
    }
    if ((n2 == jint(0)))
    {
        return;
    }
    jint n5 = jint(0);
    while ((n5 < n2))
    {
        jint n6 = n4;
        jint n7 = jint(0);
        while ((n7 < n))
        {
            jint n8 = (nArray2[n6] & jint(0xFCFCFF));
            jint n9 = (nArray[n6] & jint(0xFCFCFF));
            (nArray[(n6++)] = (jint(0xFF000000) | ((n8 + n9) >> jint(1))));
            (++n7);
        }
        (n4 += n3);
        (++n5);
    }
}

// raycasting.Position3D — reconstructed Java line 1343
void raycasting::Position3D(double d, double d2, double d3, double d4, double d5, double d6, double d7, Ref<kRenderContext> kRenderContext2) {
    if ((!kRenderContext2->drawable))
    {
        return;
    }
    double d8 = std::cos(d4);
    double d9 = std::sin(d4);
    double d10 = std::cos(d5);
    double d11 = std::sin(d5);
    double d12 = std::cos(d6);
    double d13 = std::sin(d6);
    (kRenderContext2->PosX = d);
    (kRenderContext2->PosY = d2);
    (kRenderContext2->PosZ = d3);
    jint n = jint(0);
    while ((n < kRenderContext2->MaxPreY))
    {
        jint n2 = jint(0);
        while ((n2 < kRenderContext2->MaxPreX))
        {
            double d14 = std::static_pointer_cast<kRenderContext>(kRenderContext2)->Pretable[n2][n]->XI;
            double d15 = std::static_pointer_cast<kRenderContext>(kRenderContext2)->Pretable[n2][n]->YI;
            double d16 = ((d8 * d14) - (d9 * d15));
            double d17 = ((d9 * d14) + (d8 * d15));
            double d18 = d7;
            double d19 = d16;
            double d20 = ((d10 * d17) - (d11 * d18));
            double d21 = ((d11 * d17) + (d10 * d18));
            (std::static_pointer_cast<kRenderContext>(kRenderContext2)->Pretable[n2][n]->XF = ((d13 * d21) + (d12 * d19)));
            (std::static_pointer_cast<kRenderContext>(kRenderContext2)->Pretable[n2][n]->YF = d20);
            (std::static_pointer_cast<kRenderContext>(kRenderContext2)->Pretable[n2][n]->ZF = ((d12 * d21) - (d13 * d19)));
            (++n2);
        }
        (++n);
    }
}

// raycasting.Setzoom3D — reconstructed Java line 1377
void raycasting::Setzoom3D(double d, double d2, double d3, double d4, double d5, double d6, Ref<kRenderContext> kRenderContext2, Ref<kTexture> kTexture2, double d7) {
    double d8{};
    double d9{};
    double d10{};
    if ((!kRenderContext2->drawable))
    {
        return;
    }
    double d11 = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    double d12 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    double d13 = (d6 / d3);
    if ((d3 <= 0.01))
    {
        return;
    }
    double d14 = (double(kTexture2->Modulo) / d11);
    double d15 = (double(kTexture2->Height) / d12);
    double d16 = (d10 = (d * d13));
    double d17 = (d9 = (d2 * d13));
    double d18 = 0.0;
    double d19 = 0.0;
    double d20 = (kTexture2->Modulo - jint(1));
    double d21 = (kTexture2->Height - jint(1));
    (d14 = (((d14 * 0.5) * d4) * d13));
    (d15 = (((d15 * 0.5) * d5) * d13));
    (d10 -= d14);
    (d9 -= d15);
    (d16 += d14);
    (d17 += d15);
    (d10 = (((d10 * d11) + double(kRenderContext2->ClipX1)) + (d11 * 0.5)));
    (d9 = (((d9 * d12) + double(kRenderContext2->ClipY1)) + (d12 * 0.5)));
    (d16 = (((d16 * d11) + double(kRenderContext2->ClipX1)) + (d11 * 0.5)));
    (d17 = (((d17 * d12) + double(kRenderContext2->ClipY1)) + (d12 * 0.5)));
    if ((d10 > d16))
    {
        (d8 = d16);
        (d16 = d10);
        (d10 = d8);
        (d8 = d20);
        (d20 = d18);
        (d18 = d8);
    }
    if ((d9 > d17))
    {
        (d8 = d17);
        (d17 = d9);
        (d9 = d8);
        (d8 = d21);
        (d21 = d19);
        (d19 = d8);
    }
    if ((d16 > double(kRenderContext2->ClipX2)))
    {
        (d20 = (((double(kRenderContext2->ClipX2) - d10) * (d20 - d18)) / (d16 - d10)));
        (d16 = kRenderContext2->ClipX2);
    }
    if ((d10 < double(kRenderContext2->ClipX1)))
    {
        (d18 = (((double(kRenderContext2->ClipX1) - d10) * (d20 - d18)) / (d16 - d10)));
        (d10 = kRenderContext2->ClipX1);
    }
    if ((d17 > double(kRenderContext2->ClipY2)))
    {
        (d21 = (((double(kRenderContext2->ClipY2) - d9) * (d21 - d19)) / (d17 - d9)));
        (d17 = kRenderContext2->ClipY2);
    }
    if ((d9 < double(kRenderContext2->ClipY1)))
    {
        (d19 = (((double(kRenderContext2->ClipY1) - d9) * (d21 - d19)) / (d17 - d9)));
        (d9 = kRenderContext2->ClipY1);
    }
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(1)]->u = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(0)]->u = jint((d18 * 65536.0))));
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(1)]->u = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(0)]->u = jint((d20 * 65536.0))));
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(0)]->v = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(0)]->v = jint((d19 * 65536.0))));
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(1)]->v = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(1)]->v = jint((d21 * 65536.0))));
    this->OneRec->SetSub(kRenderContext2, jint(d10), jint(d9), jint(d16), jint(d17));
    this->drawrecUVSA(kTexture2, this->OneRec, this->Rootrc->Pretable[jint(0)][jint(0)], this->Rootrc->Pretable[jint(1)][jint(0)], this->Rootrc->Pretable[jint(0)][jint(1)], this->Rootrc->Pretable[jint(1)][jint(1)], jint(d7));
}

// raycasting.SetzoomP — reconstructed Java line 1448
void raycasting::SetzoomP(double d, double d2, double d3, double d4, Ref<kRenderContext> kRenderContext2, Ref<kTexture> kTexture2, double d5) {
    double d6{};
    if ((!kRenderContext2->drawable))
    {
        return;
    }
    double d7 = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    double d8 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    double d9 = 0.0;
    double d10 = 0.0;
    double d11 = (kTexture2->Modulo - jint(1));
    double d12 = (kTexture2->Height - jint(1));
    (d = ((d * d7) + double(kRenderContext2->ClipX1)));
    (d2 = ((d2 * d8) + double(kRenderContext2->ClipY1)));
    (d3 = ((d3 * d7) + double(kRenderContext2->ClipX1)));
    (d4 = ((d4 * d8) + double(kRenderContext2->ClipY1)));
    if ((d > d3))
    {
        (d6 = d3);
        (d3 = d);
        (d = d6);
        (d6 = d11);
        (d11 = d9);
        (d9 = d6);
    }
    if ((d2 > d4))
    {
        (d6 = d4);
        (d4 = d2);
        (d2 = d6);
        (d6 = d12);
        (d12 = d10);
        (d10 = d6);
    }
    if ((d3 > double(kRenderContext2->ClipX2)))
    {
        (d11 = (((double(kRenderContext2->ClipX2) - d) * (d11 - d9)) / (d3 - d)));
        (d3 = kRenderContext2->ClipX2);
    }
    if ((d < double(kRenderContext2->ClipX1)))
    {
        (d9 = (((double(kRenderContext2->ClipX1) - d) * (d11 - d9)) / (d3 - d)));
        (d = kRenderContext2->ClipX1);
    }
    if ((d4 > double(kRenderContext2->ClipY2)))
    {
        (d12 = (((double(kRenderContext2->ClipY2) - d2) * (d12 - d10)) / (d4 - d2)));
        (d4 = kRenderContext2->ClipY2);
    }
    if ((d2 < double(kRenderContext2->ClipY1)))
    {
        (d10 = (((double(kRenderContext2->ClipY1) - d2) * (d12 - d10)) / (d4 - d2)));
        (d2 = kRenderContext2->ClipY1);
    }
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(1)]->u = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(0)]->u = jint((d9 * 65536.0))));
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(1)]->u = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(0)]->u = jint((d11 * 65536.0))));
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(0)]->v = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(0)]->v = jint((d10 * 65536.0))));
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(1)]->v = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(1)]->v = jint((d12 * 65536.0))));
    this->OneRec->SetSub(kRenderContext2, jint(d), jint(d2), jint(d3), jint(d4));
    this->drawrecUVSA(kTexture2, this->OneRec, this->Rootrc->Pretable[jint(0)][jint(0)], this->Rootrc->Pretable[jint(1)][jint(0)], this->Rootrc->Pretable[jint(0)][jint(1)], this->Rootrc->Pretable[jint(1)][jint(1)], jint(d5));
}

// raycasting.colorclip — reconstructed Java line 1503
void raycasting::colorclip(Ref<helpfade> helpfade2, Ref<kPixel> kPixel2, double d) {
    (kPixel2->r = jint(helpfade2->Frv1));
    if ((d > helpfade2->Frd2))
    {
        (kPixel2->r = jint(helpfade2->Frv2));
    }
    else
    if ((d > helpfade2->Frd1))
    {
        (kPixel2->r = jint((helpfade2->Frv1 + (((helpfade2->Frv2 - helpfade2->Frv1) * (d - helpfade2->Frd1)) / (helpfade2->Frd2 - helpfade2->Frd1)))));
    }
    (kPixel2->g = jint(helpfade2->Fgv1));
    if ((d > helpfade2->Fgd2))
    {
        (kPixel2->g = jint(helpfade2->Fgv2));
    }
    else
    if ((d > helpfade2->Fgd1))
    {
        (kPixel2->g = jint((helpfade2->Fgv1 + (((helpfade2->Fgv2 - helpfade2->Fgv1) * (d - helpfade2->Fgd1)) / (helpfade2->Fgd2 - helpfade2->Fgd1)))));
    }
    (kPixel2->b = jint(helpfade2->Fbv1));
    if ((d > helpfade2->Fbd2))
    {
        (kPixel2->b = jint(helpfade2->Fbv2));
    }
    else
    if ((d > helpfade2->Fbd1))
    {
        (kPixel2->b = jint((helpfade2->Fbv1 + (((helpfade2->Fbv2 - helpfade2->Fbv1) * (d - helpfade2->Fbd1)) / (helpfade2->Fbd2 - helpfade2->Fbd1)))));
    }
}

// raycasting.draw — reconstructed Java line 1524
void raycasting::draw(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2) {
    jint n = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    jint n2 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    jint n3 = jint(0);
    jint n4 = jint(0);
    if ((!kRenderContext2->drawable))
    {
        return;
    }
    jint n5 = jint(0);
    while ((n5 < n2))
    {
        (n3 = (((n5 + jint(16)) < n2) ? jint(16) : (n2 - n5)));
        jint n6 = jint(0);
        while ((n6 < n))
        {
            (n4 = (((n6 + jint(16)) < n) ? jint(16) : (n - n6)));
            this->OneRec->SetSub(kRenderContext2, (n6 + kRenderContext2->ClipX1), (n5 + kRenderContext2->ClipY1), ((n6 + n4) + kRenderContext2->ClipX1), ((n5 + n3) + kRenderContext2->ClipY1));
            this->drawrecUVRGB(kTexture2, this->OneRec, kRenderContext2->Pretable[(n6 >> jint(4))][(n5 >> jint(4))], kRenderContext2->Pretable[((n6 >> jint(4)) + jint(1))][(n5 >> jint(4))], kRenderContext2->Pretable[(n6 >> jint(4))][((n5 >> jint(4)) + jint(1))], kRenderContext2->Pretable[((n6 >> jint(4)) + jint(1))][((n5 >> jint(4)) + jint(1))]);
            (n6 += jint(16));
        }
        (n5 += jint(16));
    }
}

// raycasting.drawrecUV — reconstructed Java line 1546
void raycasting::drawrecUV(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5) {
    jint n = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    jint n2 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    jint n3 = kRenderContext2->Modulo;
    jint n4 = (kRenderContext2->ClipX1 + (kRenderContext2->ClipY1 * kRenderContext2->Modulo));
    Array<jint> nArray = std::static_pointer_cast<kRenderContext>(kRenderContext2)->RC_RootContext->CurrentChunk;
    Array<jint> nArray2 = kTexture2->pixels;
    jint n5 = kPixel2->u;
    jint n6 = kPixel2->v;
    jint n7 = ((kPixel4->u - n5) / n2);
    jint n8 = ((kPixel4->v - n6) / n2);
    jint n9 = kPixel3->u;
    jint n10 = kPixel3->v;
    jint n11 = ((kPixel5->u - n9) / n2);
    jint n12 = ((kPixel5->v - n10) / n2);
    jint n13 = jint(0);
    while ((n13 < n2))
    {
        jint n14 = n4;
        jint n15 = n5;
        jint n16 = n6;
        jint n17 = ((n9 - n15) / n);
        jint n18 = ((n10 - n16) / n);
        jint n19 = jint(0);
        while ((n19 < n))
        {
            (nArray[(n14++)] = nArray2[(((n16 >> jint(8)) & jint(0xFF00)) | ((n15 >> jint(16)) & jint(0xFF)))]);
            (n15 += n17);
            (n16 += n18);
            (++n19);
        }
        (n5 += n7);
        (n6 += n8);
        (n9 += n11);
        (n10 += n12);
        (n4 += n3);
        (++n13);
    }
}

// raycasting.drawrecUVL — reconstructed Java line 1584
void raycasting::drawrecUVL(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5) {
    jint n = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    jint n2 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    jint n3 = kRenderContext2->Modulo;
    jint n4 = (kRenderContext2->ClipX1 + (kRenderContext2->ClipY1 * kRenderContext2->Modulo));
    Array<jint> nArray = std::static_pointer_cast<kRenderContext>(kRenderContext2)->RC_RootContext->CurrentChunk;
    Array<jint> nArray2 = kTexture2->pixels;
    jint n5 = jint(0);
    jint n6 = jint(255);
    jint n7 = kPixel2->u;
    jint n8 = kPixel2->v;
    jint n9 = kPixel2->r;
    jint n10 = ((kPixel4->u - n7) / n2);
    jint n11 = ((kPixel4->v - n8) / n2);
    jint n12 = ((kPixel4->r - n9) / n2);
    jint n13 = kPixel3->u;
    jint n14 = kPixel3->v;
    jint n15 = kPixel3->r;
    jint n16 = ((kPixel5->u - n13) / n2);
    jint n17 = ((kPixel5->v - n14) / n2);
    jint n18 = ((kPixel5->r - n15) / n2);
    jint n19 = jint(0);
    while ((n19 < n2))
    {
        jint n20 = n4;
        jint n21 = n7;
        jint n22 = n8;
        jint n23 = n9;
        jint n24 = ((n13 - n21) / n);
        jint n25 = ((n14 - n22) / n);
        jint n26 = ((n15 - n23) / n);
        jint n27 = jint(0);
        while ((n27 < n))
        {
            jint n28 = nArray2[(((n22 >> jint(8)) & jint(0xFF00)) | ((n21 >> jint(16)) & jint(0xFF)))];
            jint n29 = (n23 >> jint(16));
            jint n30 = (((n28 & jint(0xFF0000)) >> jint(16)) + n29);
            jint n31 = (((n28 & jint(0xFF00)) >> jint(8)) + n29);
            (n28 = ((n28 & jint(0xFF)) + n29));
            if ((n30 > n6))
            {
                (n30 = n6);
            }
            else
            if ((n30 < n5))
            {
                (n30 = n5);
            }
            if ((n31 > n6))
            {
                (n31 = n6);
            }
            else
            if ((n31 < n5))
            {
                (n31 = n5);
            }
            if ((n28 > n6))
            {
                (n28 = n6);
            }
            else
            if ((n28 < n5))
            {
                (n28 = n5);
            }
            (nArray[(n20++)] = (((jint(0xFF000000) | (n30 << jint(16))) | (n31 << jint(8))) | n28));
            (n21 += n24);
            (n22 += n25);
            (n23 += n26);
            (++n27);
        }
        (n7 += n10);
        (n8 += n11);
        (n9 += n12);
        (n13 += n16);
        (n14 += n17);
        (n15 += n18);
        (n4 += n3);
        (++n19);
    }
}

// raycasting.drawrecUVM — reconstructed Java line 1653
void raycasting::drawrecUVM(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5) {
    jint n = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    jint n2 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    jint n3 = kRenderContext2->Modulo;
    jint n4 = (kRenderContext2->ClipX1 + (kRenderContext2->ClipY1 * kRenderContext2->Modulo));
    Array<jint> nArray = std::static_pointer_cast<kRenderContext>(kRenderContext2)->RC_RootContext->CurrentChunk;
    Array<jint> nArray2 = kTexture2->pixels;
    bool bl = false;
    jint n5 = jint(255);
    jint n6 = kTexture2->Modulo;
    if ((n == jint(0)))
    {
        return;
    }
    if ((n2 == jint(0)))
    {
        return;
    }
    jint n7 = kPixel2->u;
    jint n8 = kPixel2->v;
    jint n9 = ((kPixel4->u - n7) / n2);
    jint n10 = ((kPixel4->v - n8) / n2);
    jint n11 = kPixel3->u;
    jint n12 = kPixel3->v;
    jint n13 = ((kPixel5->u - n11) / n2);
    jint n14 = ((kPixel5->v - n12) / n2);
    jint n15 = jint(0);
    while ((n15 < n2))
    {
        jint n16 = n4;
        jint n17 = n7;
        jint n18 = n8;
        jint n19 = ((n11 - n17) / n);
        jint n20 = ((n12 - n18) / n);
        jint n21 = jint(0);
        while ((n21 < n))
        {
            jint n22 = nArray2[(((n18 >> jint(16)) * n6) + (n17 >> jint(16)))];
            jint n23 = nArray[n16];
            jint n24 = (((n22 & jint(0xFF0000)) >> jint(16)) + ((n23 & jint(0xFF0000)) >> jint(16)));
            jint n25 = (((n22 & jint(0xFF00)) >> jint(8)) + ((n23 & jint(0xFF00)) >> jint(8)));
            (n22 = ((n22 & jint(0xFF)) + (n23 & jint(0xFF))));
            if ((n24 > n5))
            {
                (n24 = n5);
            }
            if ((n25 > n5))
            {
                (n25 = n5);
            }
            if ((n22 > n5))
            {
                (n22 = n5);
            }
            (nArray[(n16++)] = (((jint(0xFF000000) | (n24 << jint(16))) | (n25 << jint(8))) | n22));
            (n17 += n19);
            (n18 += n20);
            (++n21);
        }
        (n7 += n9);
        (n8 += n10);
        (n11 += n13);
        (n12 += n14);
        (n4 += n3);
        (++n15);
    }
}

// raycasting.drawrecUVRGB — reconstructed Java line 1714
void raycasting::drawrecUVRGB(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5) {
    jint n = jint(0);
    jint n2 = jint(255);
    jint n3 = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    jint n4 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    jint n5 = kRenderContext2->Modulo;
    jint n6 = (kRenderContext2->ClipX1 + (kRenderContext2->ClipY1 * kRenderContext2->Modulo));
    Array<jint> nArray = std::static_pointer_cast<kRenderContext>(kRenderContext2)->RC_RootContext->CurrentChunk;
    Array<jint> nArray2 = kTexture2->pixels;
    jint n7 = kPixel2->u;
    jint n8 = kPixel2->v;
    jint n9 = kPixel2->r;
    jint n10 = kPixel2->g;
    jint n11 = kPixel2->b;
    jint n12 = ((kPixel4->u - n7) / n4);
    jint n13 = ((kPixel4->v - n8) / n4);
    jint n14 = ((kPixel4->r - n9) / n4);
    jint n15 = ((kPixel4->g - n10) / n4);
    jint n16 = ((kPixel4->b - n11) / n4);
    jint n17 = kPixel3->u;
    jint n18 = kPixel3->v;
    jint n19 = kPixel3->r;
    jint n20 = kPixel3->g;
    jint n21 = kPixel3->b;
    jint n22 = ((kPixel5->u - n17) / n4);
    jint n23 = ((kPixel5->v - n18) / n4);
    jint n24 = ((kPixel5->r - n19) / n4);
    jint n25 = ((kPixel5->g - n20) / n4);
    jint n26 = ((kPixel5->b - n21) / n4);
    jint n27 = jint(0);
    while ((n27 < n4))
    {
        jint n28 = n6;
        jint n29 = n7;
        jint n30 = n8;
        jint n31 = n9;
        jint n32 = n10;
        jint n33 = n11;
        jint n34 = ((n17 - n29) / n3);
        jint n35 = ((n18 - n30) / n3);
        jint n36 = ((n19 - n31) / n3);
        jint n37 = ((n20 - n32) / n3);
        jint n38 = ((n21 - n33) / n3);
        jint n39 = jint(0);
        while ((n39 < n3))
        {
            jint n40 = nArray2[(((n30 >> jint(8)) & jint(0xFF00)) | ((n29 >> jint(16)) & jint(0xFF)))];
            jint n41 = ((n31 & jint(0xFFFF0000)) >> jint(16));
            jint n42 = ((n32 & jint(0xFFFF0000)) >> jint(16));
            jint n43 = ((n33 & jint(0xFFFF0000)) >> jint(16));
            jint n44 = (((n40 & jint(0xFF0000)) >> jint(16)) + n41);
            jint n45 = (((n40 & jint(0xFF00)) >> jint(8)) + n42);
            (n40 = ((n40 & jint(0xFF)) + n43));
            if ((n44 > n2))
            {
                (n44 = n2);
            }
            else
            if ((n44 < n))
            {
                (n44 = n);
            }
            if ((n45 > n2))
            {
                (n45 = n2);
            }
            else
            if ((n45 < n))
            {
                (n45 = n);
            }
            if ((n40 > n2))
            {
                (n40 = n2);
            }
            else
            if ((n40 < n))
            {
                (n40 = n);
            }
            (nArray[(n28++)] = (((jint(0xFF000000) | (n44 << jint(16))) | (n45 << jint(8))) | n40));
            (n29 += n34);
            (n30 += n35);
            (n31 += n36);
            (n32 += n37);
            (n33 += n38);
            (++n39);
        }
        (n7 += n12);
        (n8 += n13);
        (n9 += n14);
        (n10 += n15);
        (n11 += n16);
        (n17 += n22);
        (n18 += n23);
        (n19 += n24);
        (n20 += n25);
        (n21 += n26);
        (n6 += n5);
        (++n27);
    }
}

// raycasting.drawrecUVS — reconstructed Java line 1803
void raycasting::drawrecUVS(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5) {
    jint n = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    jint n2 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    jint n3 = kRenderContext2->Modulo;
    jint n4 = (kRenderContext2->ClipX1 + (kRenderContext2->ClipY1 * kRenderContext2->Modulo));
    Array<jint> nArray = std::static_pointer_cast<kRenderContext>(kRenderContext2)->RC_RootContext->CurrentChunk;
    Array<jint> nArray2 = kTexture2->pixels;
    jint n5 = kTexture2->Modulo;
    if ((n == jint(0)))
    {
        return;
    }
    if ((n2 == jint(0)))
    {
        return;
    }
    jint n6 = kPixel2->u;
    jint n7 = kPixel2->v;
    jint n8 = ((kPixel4->u - n6) / n2);
    jint n9 = ((kPixel4->v - n7) / n2);
    jint n10 = kPixel3->u;
    jint n11 = kPixel3->v;
    jint n12 = ((kPixel5->u - n10) / n2);
    jint n13 = ((kPixel5->v - n11) / n2);
    jint n14 = jint(0);
    while ((n14 < n2))
    {
        jint n15 = n4;
        jint n16 = n6;
        jint n17 = n7;
        jint n18 = ((n10 - n16) / n);
        jint n19 = ((n11 - n17) / n);
        jint n20 = jint(0);
        while ((n20 < n))
        {
            jint n21 = nArray2[(((n17 >> jint(16)) * n5) + (n16 >> jint(16)))];
            if ((n21 != (-jint(1))))
            {
                (nArray[n15] = n21);
            }
            (++n15);
            (n16 += n18);
            (n17 += n19);
            (++n20);
        }
        (n6 += n8);
        (n7 += n9);
        (n10 += n12);
        (n11 += n13);
        (n4 += n3);
        (++n14);
    }
}

// raycasting.drawrecUVSA — reconstructed Java line 1852
void raycasting::drawrecUVSA(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5, jint n) {
    if ((n <= jint(0)))
    {
        return;
    }
    if ((n >= jint(255)))
    {
        this->drawrecUVS(kTexture2, kRenderContext2, kPixel2, kPixel3, kPixel4, kPixel5);
        return;
    }
    jint n2 = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    jint n3 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    jint n4 = kRenderContext2->Modulo;
    jint n5 = (kRenderContext2->ClipX1 + (kRenderContext2->ClipY1 * kRenderContext2->Modulo));
    Array<jint> nArray = std::static_pointer_cast<kRenderContext>(kRenderContext2)->RC_RootContext->CurrentChunk;
    Array<jint> nArray2 = kTexture2->pixels;
    jint n6 = kTexture2->Modulo;
    jint n7 = (jint(255) - n);
    if ((n2 == jint(0)))
    {
        return;
    }
    if ((n3 == jint(0)))
    {
        return;
    }
    jint n8 = kPixel2->u;
    jint n9 = kPixel2->v;
    jint n10 = ((kPixel4->u - n8) / n3);
    jint n11 = ((kPixel4->v - n9) / n3);
    jint n12 = kPixel3->u;
    jint n13 = kPixel3->v;
    jint n14 = ((kPixel5->u - n12) / n3);
    jint n15 = ((kPixel5->v - n13) / n3);
    jint n16 = jint(0);
    while ((n16 < n3))
    {
        jint n17 = n5;
        jint n18 = n8;
        jint n19 = n9;
        jint n20 = ((n12 - n18) / n2);
        jint n21 = ((n13 - n19) / n2);
        jint n22 = jint(0);
        while ((n22 < n2))
        {
            jint n23 = nArray2[(((n19 >> jint(16)) * n6) + (n18 >> jint(16)))];
            if ((n23 != (-jint(1))))
            {
                jint n24 = nArray[n17];
                jint n25 = ((n23 & jint(0xFF0000)) >> jint(16));
                jint n26 = ((n23 & jint(0xFF00)) >> jint(8));
                (n23 &= jint(0xFF));
                jint n27 = ((n24 & jint(0xFF0000)) >> jint(16));
                jint n28 = ((n24 & jint(0xFF00)) >> jint(8));
                (n24 &= jint(0xFF));
                (n27 = (((n27 * n7) + (n25 * n)) >> jint(8)));
                (n28 = (((n28 * n7) + (n26 * n)) >> jint(8)));
                (n24 = (((n24 * n7) + (n23 * n)) >> jint(8)));
                (nArray[n17] = (((jint(0xFF000000) | (n27 << jint(16))) | (n28 << jint(8))) | n24));
            }
            (++n17);
            (n18 += n20);
            (n19 += n21);
            (++n22);
        }
        (n8 += n10);
        (n9 += n11);
        (n12 += n14);
        (n13 += n15);
        (n5 += n4);
        (++n16);
    }
}

// raycasting.findfloor — reconstructed Java line 1919
void raycasting::findfloor(double d, double d2, double d3, double d4, Ref<kRenderContext> kRenderContext2, Ref<helpfade> helpfade2) {
    if ((!kRenderContext2->drawable))
    {
        return;
    }
    double d5 = 65536.0;
    (helpfade2->Frv1 *= d5);
    (helpfade2->Frv2 *= d5);
    (helpfade2->Fgv1 *= d5);
    (helpfade2->Fgv2 *= d5);
    (helpfade2->Fbv1 *= d5);
    (helpfade2->Fbv2 *= d5);
    (d3 *= 65536.0);
    (d4 *= 65536.0);
    (d += kRenderContext2->PosY);
    (d2 -= kRenderContext2->PosY);
    jint n = jint(0);
    while ((n < kRenderContext2->MaxPreY))
    {
        jint n2 = jint(0);
        while ((n2 < kRenderContext2->MaxPreX))
        {
            double d6{};
            double d7{};
            double d8{};
            Ref<kPixel> kPixel2 = kRenderContext2->Pretable[n2][n];
            double d9 = kPixel2->XF;
            double d10 = kPixel2->YF;
            double d11 = kPixel2->ZF;
            if ((d10 == 0.0))
            {
                (d10 = 1.0E-8);
            }
            double d12 = (1.0 / d10);
            if ((d10 < 0.0))
            {
                (d8 = ((d11 * d) * d12));
                (d7 = ((d9 * d) * d12));
                (kPixel2->u = jint(((d8 - kRenderContext2->PosZ) * d3)));
                (kPixel2->v = jint(((d7 - kRenderContext2->PosX) * d4)));
                (d6 = std::sqrt((((d8 * d8) + (d7 * d7)) + (d * d))));
                this->colorclip(helpfade2, kPixel2, d6);
            }
            else
            {
                (d8 = ((d11 * d2) * d12));
                (d7 = ((d9 * d2) * d12));
                (kPixel2->u = jint((((-d8) - kRenderContext2->PosZ) * d3)));
                (kPixel2->v = jint((((-d7) - kRenderContext2->PosX) * d4)));
                (d6 = std::sqrt((((d8 * d8) + (d7 * d7)) + (d2 * d2))));
                this->colorclip(helpfade2, kPixel2, d6);
            }
            (++n2);
        }
        (++n);
    }
}

// raycasting.findtunnel — reconstructed Java line 1970
void raycasting::findtunnel(double d, double d2, Ref<kRenderContext> kRenderContext2, Ref<helpfade> helpfade2) {
    if ((!kRenderContext2->drawable))
    {
        return;
    }
    double d3 = 65536.0;
    (helpfade2->Frv1 *= d3);
    (helpfade2->Frv2 *= d3);
    (helpfade2->Fgv1 *= d3);
    (helpfade2->Fgv2 *= d3);
    (helpfade2->Fbv1 *= d3);
    (helpfade2->Fbv2 *= d3);
    (d2 *= 65536.0);
    double d4 = 5340353.715440872;
    jint n = jint(0);
    while ((n < kRenderContext2->MaxPreY))
    {
        jint n2 = jint(0);
        while ((n2 < kRenderContext2->MaxPreX))
        {
            Ref<kPixel> kPixel2 = kRenderContext2->Pretable[n2][n];
            double d5 = kPixel2->XF;
            double d6 = std::fabs(kPixel2->YF);
            double d7 = kPixel2->ZF;
            double d8 = std::sqrt(((d5 * d5) + (d6 * d6)));
            if ((d8 == 0.0))
            {
                (d8 = 1.0E-7);
            }
            double d9 = std::atan2(d5, d6);
            double d10 = (std::cos(d9) * d);
            double d11 = (std::sin(d9) * d);
            double d12 = ((d7 * d) / d8);
            (kPixel2->u = jint(((d12 + kRenderContext2->PosZ) * d2)));
            (kPixel2->v = (jint((d9 * d4)) + jint(0x800000)));
            (d8 = std::sqrt((((d12 * d12) + (d10 * d10)) + (d11 * d11))));
            this->colorclip(helpfade2, kPixel2, d8);
            (++n2);
        }
        (++n);
    }
}

// raycasting.findtwirl — reconstructed Java line 2009
void raycasting::findtwirl(double d, double d2, double d3, double d4, double d5, double d6, Ref<kRenderContext> kRenderContext2) {
    if ((!kRenderContext2->drawable))
    {
        return;
    }
    double d7 = 5.0;
    double d8 = 750.0;
    double d9 = (d8 - d7);
    jint n = (jint((kRenderContext2->PosZ * 65536.0)) & jint(0xFFFFFF));
    double d10 = 5340353.715440872;
    jint n2 = jint(0);
    while ((n2 < kRenderContext2->MaxPreY))
    {
        jint n3 = jint(0);
        while ((n3 < kRenderContext2->MaxPreX))
        {
            Ref<kPixel> kPixel2 = kRenderContext2->Pretable[n3][n2];
            double d11 = (kPixel2->XF * 256.0);
            double d12 = (kPixel2->YF * 256.0);
            double d13 = std::atan2(d11, d12);
            double d14 = std::sqrt(((d11 * d11) + (d12 * d12)));
            (d13 += (d3 * std::sin(((d + ((d14 * 3.141592653589793) / 128.0)) * d2))));
            (d14 += (d6 * std::sin(((d4 + ((d14 * 3.141592653589793) / 128.0)) * d5))));
            (kPixel2->u = jint((((std::sin(d13) * d14) * 65536.0) + (kRenderContext2->PosX * 65536.0))));
            (kPixel2->v = jint((((std::cos(d13) * d14) * 65536.0) + (kRenderContext2->PosY * 65536.0))));
            double d15 = (d12 - 8.0);
            (d13 = std::atan2(d11, d15));
            (d14 = std::sqrt(((d11 * d11) + (d15 * d15))));
            (d13 += (d3 * std::sin(((d + ((d14 * 3.141592653589793) / 128.0)) * d2))));
            (d14 += (d6 * std::sin(((d4 + ((d14 * 3.141592653589793) / 128.0)) * d5))));
            jint n4 = (kPixel2->u - jint((((std::sin(d13) * d14) * 65536.0) + (kRenderContext2->PosX * 65536.0))));
            (kPixel2->r = (n4 * jint(26)));
            (kPixel2->g = (n4 * jint(24)));
            (kPixel2->b = (n4 * jint(23)));
            (++n3);
        }
        (++n2);
    }
}

// raycasting.flare3D — reconstructed Java line 2046
void raycasting::flare3D(double d, double d2, double d3, double d4, double d5, double d6, Ref<kRenderContext> kRenderContext2, Ref<kTexture> kTexture2) {
    double d7{};
    double d8{};
    double d9{};
    if ((!kRenderContext2->drawable))
    {
        return;
    }
    double d10 = (kRenderContext2->ClipX2 - kRenderContext2->ClipX1);
    double d11 = (kRenderContext2->ClipY2 - kRenderContext2->ClipY1);
    double d12 = (d6 / d3);
    if ((d3 <= 0.01))
    {
        return;
    }
    double d13 = (double(kTexture2->Modulo) / d10);
    double d14 = (double(kTexture2->Height) / d11);
    double d15 = (d9 = (d * d12));
    double d16 = (d8 = (d2 * d12));
    double d17 = 0.0;
    double d18 = 0.0;
    double d19 = (kTexture2->Modulo - jint(1));
    double d20 = (kTexture2->Height - jint(1));
    (d13 = (((d13 * 0.5) * d4) * d12));
    (d14 = (((d14 * 0.5) * d5) * d12));
    (d9 -= d13);
    (d8 -= d14);
    (d15 += d13);
    (d16 += d14);
    (d9 = (((d9 * d10) + double(kRenderContext2->ClipX1)) + (d10 * 0.5)));
    (d8 = (((d8 * d11) + double(kRenderContext2->ClipY1)) + (d11 * 0.5)));
    (d15 = (((d15 * d10) + double(kRenderContext2->ClipX1)) + (d10 * 0.5)));
    (d16 = (((d16 * d11) + double(kRenderContext2->ClipY1)) + (d11 * 0.5)));
    if ((d9 > d15))
    {
        (d7 = d15);
        (d15 = d9);
        (d9 = d7);
        (d7 = d19);
        (d19 = d17);
        (d17 = d7);
    }
    if ((d8 > d16))
    {
        (d7 = d16);
        (d16 = d8);
        (d8 = d7);
        (d7 = d20);
        (d20 = d18);
        (d18 = d7);
    }
    if ((d15 > double(kRenderContext2->ClipX2)))
    {
        (d19 = (((double(kRenderContext2->ClipX2) - d9) * (d19 - d17)) / (d15 - d9)));
        (d15 = kRenderContext2->ClipX2);
    }
    if ((d9 < double(kRenderContext2->ClipX1)))
    {
        (d17 = (((double(kRenderContext2->ClipX1) - d9) * (d19 - d17)) / (d15 - d9)));
        (d9 = kRenderContext2->ClipX1);
    }
    if ((d16 > double(kRenderContext2->ClipY2)))
    {
        (d20 = (((double(kRenderContext2->ClipY2) - d8) * (d20 - d18)) / (d16 - d8)));
        (d16 = kRenderContext2->ClipY2);
    }
    if ((d8 < double(kRenderContext2->ClipY1)))
    {
        (d18 = (((double(kRenderContext2->ClipY1) - d8) * (d20 - d18)) / (d16 - d8)));
        (d8 = kRenderContext2->ClipY1);
    }
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(1)]->u = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(0)]->u = jint((d17 * 65536.0))));
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(1)]->u = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(0)]->u = jint((d19 * 65536.0))));
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(0)]->v = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(0)]->v = jint((d18 * 65536.0))));
    (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(1)][jint(1)]->v = (std::static_pointer_cast<kRenderContext>(this->Rootrc)->Pretable[jint(0)][jint(1)]->v = jint((d20 * 65536.0))));
    this->OneRec->SetSub(kRenderContext2, jint(d9), jint(d8), jint(d15), jint(d16));
    this->drawrecUVM(kTexture2, this->OneRec, this->Rootrc->Pretable[jint(0)][jint(0)], this->Rootrc->Pretable[jint(1)][jint(0)], this->Rootrc->Pretable[jint(0)][jint(1)], this->Rootrc->Pretable[jint(1)][jint(1)]);
}

// kSound.kSound — reconstructed Java line 13
kSound::kSound(kraycasting* owner, Ref<DemoAudio> audioClip) : EngineObject(owner) {
    (this->OurAudio = audioClip);
    (this->soundIsOn = false);
    (this->nextkSound = raw(owner->firstkSound));
    (owner->firstkSound = raw(this));
}

// kSound.LoopOne — reconstructed Java line 20
void kSound::LoopOne() {
    if ((this->OurAudio != nullptr))
    {
        this->OurAudio->loop();
        (this->soundIsOn = true);
    }
}

// kSound.PlayOne — reconstructed Java line 27
void kSound::PlayOne() {
    if ((this->OurAudio != nullptr))
    {
        this->OurAudio->play();
        (this->soundIsOn = false);
        (this->playedonce = true);
    }
}

// kSound.StopAll — reconstructed Java line 35
void kSound::StopAll() {
    kSound* kSound2 = owner->firstkSound;
    while ((kSound2 != nullptr))
    {
        kSound2->StopOne();
        (kSound2 = kSound2->nextkSound);
    }
}

// kSound.StopOne — reconstructed Java line 43
void kSound::StopOne() {
    if ((this->OurAudio != nullptr))
    {
        this->OurAudio->stop();
        (this->soundIsOn = false);
    }
}

// A single original run-loop iteration, suitable for a future browser callback.
Array<jint> kraycasting::frameAt(jint tick) {
    auto part = THEkScript->part(tick);
    part->rendah(tick);
    applet_image = RenderContext->createImage();
    return applet_image;
}
Array<jint> kraycasting::frame() { return frameAt(kTime->getTime()); }
void kraycasting::stop() {
    if (stopped) return;
    stopped = true;
    boolend = true;
    if (firstkSound) firstkSound->StopAll();
}
kraycasting::~kraycasting() { stop(); }
