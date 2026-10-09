#!/usr/bin/env python3
"""One-time, reproducible source correspondence aid (javalang==0.13.0).

Emits checked-in, readable C++11 from the verified CFR reconstruction.
Normal C++ builds need neither this script nor Java/Python. The platform
substitutions below are deliberately explicit; rendering expressions, class
and method names, branches and loop bodies stay in their original order.
"""
import json
import re
from pathlib import Path
import javalang as jl
from javalang import tree as J

ROOT = Path(__file__).resolve().parents[2]
source = (ROOT/'java-desktop/src/main/java/kraycasting.java').read_text()
original_outer = jl.parse.parse(source).types[0]
original_lines = {}
for cls in [original_outer] + [c for c in original_outer.body if isinstance(c,J.ClassDeclaration)]:
    for m in cls.body:
        if isinstance(m,(J.MethodDeclaration,J.ConstructorDeclaration)):
            original_lines[(cls.name,m.name,len(m.parameters))]=m.position.line
# Narrow, reviewable AWT/thread boundary substitutions before AST translation.
start = source.index('    public void run()')
end = source.index('    public class kTexture')
source = source[:start] + source[end:]
start = source.index('        public kTexture(Image image)')
end = source.index('\n    private class kRenderContext',start)
source = source[:start] + '''        public kTexture(Image image) {
            this.Modulo = image.width;
            this.Height = image.height;
            this.pixels = image.pixels;
        }
    }
''' + source[end:]
source = re.sub(r'^.*(?:MemoryImageSource|RC_imageMIS[12]).*\n','',source,flags=re.M)
source = source.replace('public Image createImage()', 'public int[] createImage()')
source = source.replace('        }\n    }\n\n    public class kcamera', '            return this.Chunk2;\n        }\n    }\n\n    public class kcamera')
source = source.replace('private volatile Image applet_image;', 'private int[] applet_image;')
source = re.sub(r'^.*(?:mainthread|System.out\.|\.getClass\(\)).*\n','',source,flags=re.M)
source = re.sub(r'    static /\* synthetic \*/ boolean access\$16.*?\n    }\n','',source,flags=re.S)
outer = jl.parse.parse(source).types[0]
sound = jl.parse.parse((ROOT/'java-desktop/src/main/java/kSound.java').read_text()).types[0]
classes = [outer] + [c for c in outer.body if isinstance(c,J.ClassDeclaration)] + [sound]
names = {c.name for c in classes}
current = ''
correspondence = []

def typ(t):
    if t is None: return 'void'
    s={'int':'jint','Integer':'jint','long':'int64_t','byte':'int8_t','boolean':'bool',
       'String':'String','Hashtable':'Hashtable','DesktopSurface':'kraycasting*',
       'kraycasting':'kraycasting*'}.get(t.name,t.name)
    if t.name in names-{'kraycasting'} or t.name in ('Object','Image','DemoAudio'): s=f'Ref<{s}>'
    for d in t.dimensions or []: s=f'Array<{s}>'
    return s

def args_call(target,args):
    # Java evaluates arguments left to right; C++11 does not. doubleout may
    # consume random numbers. Explicit temporaries preserve that ordering.
    a=[expr(x) for x in args]
    if len(a)>1 and any('doubleout(' in x for x in a):
        decl=' '.join(f'auto argument{i} = {v};' for i,v in enumerate(a))
        return '[&]() { '+decl+' return '+target+'('+', '.join(f'argument{i}' for i in range(len(a)))+'); }()'
    return target+'('+', '.join(a)+')'

def expr(n):
    if n is None: return ''
    if isinstance(n,J.Literal):
        s=n.value
        if s=='null': s='nullptr'
        elif s.startswith('"'): s='String('+s+')'
        elif s not in ('true','false') and (s.startswith('0x') or not re.search(r'[.eE]',s)):
            s='int64_t('+s[:-1]+')' if s.endswith('L') else 'jint('+s+')'
    elif isinstance(n,J.MemberReference):
        q=n.qualifier or ''
        if q=='Math' and n.member=='PI': s='3.141592653589793'
        else: s=(q.replace('.','->')+'->' if q else '')+n.member
        if n.member=='firstkSound': s='owner->firstkSound'
    elif isinstance(n,J.This): s='owner' if n.qualifier else 'this'
    elif isinstance(n,J.BinaryOperation): s='('+expr(n.operandl)+' '+n.operator+' '+expr(n.operandr)+')'
    elif isinstance(n,J.Assignment):
        a,b=expr(n.expressionl),expr(n.value)
        if a.endswith(('->RC_RootContext','->RC_FatherContext','->nextkSound','->lastpartplayed')) or a=='owner->firstkSound': b='raw('+b+')'
        s='('+a+' '+n.type+' '+b+')'
    elif isinstance(n,J.TernaryExpression): s='('+expr(n.condition)+' ? '+expr(n.if_true)+' : '+expr(n.if_false)+')'
    elif isinstance(n,J.Cast):
        t=typ(n.type); v=expr(n.expression)
        s=f'std::static_pointer_cast<{n.type.name}>({v})' if t.startswith('Ref<') else (f'({v})' if t=='kraycasting*' else f'{t}({v})')
    elif isinstance(n,J.MethodInvocation):
        q=n.qualifier or ''
        if q=='Math': target='owner->random.random' if n.member=='random' else 'std::'+('fabs' if n.member=='abs' else n.member)
        elif q=='System' and n.member=='currentTimeMillis': target='owner->platform.currentTimeMillis'
        else: target=(q.replace('.','->')+'->' if q else '')+n.member
        s=args_call(target,n.arguments)
    elif isinstance(n,(J.ClassCreator,J.InnerClassCreator)):
        if n.type.name in ('Integer','Double'): s=args_call('jinteger' if n.type.name=='Integer' else 'jdouble',n.arguments)
        elif n.type.name=='Hashtable': s='Hashtable()'
        else:
            who='this' if current=='kraycasting' else 'owner'
            s='std::make_shared<'+n.type.name+'>('+who+''.join(', '+expr(x) for x in n.arguments)+')'
    elif isinstance(n,J.ArrayCreator):
        t=typ(n.type); d=[expr(x) for x in n.dimensions]
        s=f'Array<{t}>({d[0]})' if len(d)==1 else f'array2<{t}>({d[0]}, {d[1]})'
    else: raise NotImplementedError(repr(n))
    for sel in getattr(n,'selectors',None) or []:
        if isinstance(sel,J.ArraySelector): s+='['+expr(sel.index)+']'
        elif isinstance(sel,J.MemberReference): s+='->'+sel.member
        elif isinstance(sel,J.MethodInvocation): s=args_call(s+'->'+sel.member,sel.arguments)
        elif isinstance(sel,J.InnerClassCreator): s=expr(sel)
        else: raise NotImplementedError(repr(sel))
    for p in reversed(getattr(n,'prefix_operators',None) or []): s='('+p+s+')'
    for p in getattr(n,'postfix_operators',None) or []: s='('+s+p+')'
    return s

def stmt(n,indent=1):
    p='    '*indent
    if isinstance(n,list): return ''.join(stmt(x,indent) for x in n)
    if isinstance(n,J.BlockStatement): return p+'{\n'+stmt(n.statements,indent+1)+p+'}\n'
    if isinstance(n,J.LocalVariableDeclaration):
        return ''.join(p+('kSound*' if current=='kSound' and n.type.name=='kSound' else typ(n.type))+' '+v.name+(' = '+expr(v.initializer) if v.initializer and not (n.type.name=='String' and isinstance(v.initializer,J.Literal) and v.initializer.value=='null') else '{}')+';\n' for v in n.declarators)
    if isinstance(n,J.StatementExpression): return p+expr(n.expression)+';\n'
    if isinstance(n,J.ReturnStatement): return p+'return'+(' '+expr(n.expression) if n.expression else '')+';\n'
    if isinstance(n,J.ContinueStatement): return p+'continue;\n'
    if isinstance(n,J.WhileStatement): return p+'while ('+expr(n.condition)+')\n'+stmt(n.body,indent)
    if isinstance(n,J.IfStatement):
        s=p+'if ('+expr(n.condition)+')\n'+stmt(n.then_statement,indent)
        if n.else_statement: s+=p+'else\n'+stmt(n.else_statement,indent)
        return s
    raise NotImplementedError(repr(n))

header=['// Translated from the verified CFR reconstruction. See tools/translate.py.\n#pragma once\n#include "platform.hpp"\n']
header += [f'struct {c.name};\n' for c in classes]
impl=['// Original expression and call order retained; platform changes documented.\n#include "engine.hpp"\n']
for c in classes:
    current=c.name
    base=c.extends.name if c.extends and c.extends.name=='kParameterX' else ('Object' if current=='kraycasting' else 'EngineObject')
    header.append(f'\nstruct {current} : {base} {{\n')
    if current=='kraycasting':
        header.append('''    Platform& platform;
    JavaRandom random;
    kSound* firstkSound = nullptr;
    bool stopped = false;
    explicit kraycasting(Platform& p, uint64_t seed) : platform(p) { random.seed(seed); }
    ~kraycasting();
    Array<jint> frame();
    Array<jint> frameAt(jint tick);
    void stop();
    String getParameter(String) { return platform.script(); }
    String getDocumentBase() { return {}; }
    Ref<Image> getImage(String, String path) { return platform.image(path); }
    Ref<DemoAudio> getAudioClip(String, String path) { return platform.audio(path); }
    struct Size { jint width = 520, height = 300; Size* operator->() { return this; } };
    Size getSize() { return {}; }
''')
    for f in c.fields:
        for v in f.declarators:
            if v.name=='firstkSound': continue
            t=typ(f.type)
            if v.name in ('RC_RootContext','RC_FatherContext','nextkSound','lastpartplayed'): t=f.type.name+'*'
            init=expr(v.initializer) if v.initializer else '{}'
            header.append('    '+t+' '+v.name+' = '+init+';\n')
    constructors=[m for m in c.body if isinstance(m,J.ConstructorDeclaration)]
    if not constructors and current!='kraycasting': header.append(f'    explicit {current}(kraycasting* p) : {base}(p) {{}}\n')
    for m in c.body:
        if not isinstance(m,(J.ConstructorDeclaration,J.MethodDeclaration)): continue
        ctor=isinstance(m,J.ConstructorDeclaration)
        rt='' if ctor else typ(m.return_type)+' '
        params=', '.join(typ(p.type)+' '+p.name for p in m.parameters)
        if ctor: params='kraycasting* owner'+(', '+params if params else '')
        abstract='abstract' in m.modifiers
        header.append('    '+('virtual ' if abstract or current.startswith('kParameterX') and not ctor else '')+rt+m.name+'('+params+')'+(' = 0' if abstract else '')+';\n')
        correspondence.append({'class':current,'method':m.name,'line_java':original_lines.get((current,m.name,len(m.parameters)),m.position.line),'native':'src/engine.cpp' if m.body is not None else 'src/engine.hpp'})
        if m.body is None: continue
        impl.append(f'\n// {current}.{m.name} — reconstructed Java line {original_lines.get((current,m.name,len(m.parameters)),m.position.line)}\n')
        impl.append(rt+current+'::'+m.name+'('+params+')'+(' : '+base+'(owner)' if ctor else '')+' {\n'+stmt(m.body)+'}\n')
    header.append('};\n')
impl.append('''
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
''')
(ROOT/'native-sdl2/src/engine.hpp').write_text(''.join(header))
(ROOT/'native-sdl2/src/engine.cpp').write_text(''.join(impl))
(ROOT/'native-sdl2/tools/method-correspondence.json').write_text(json.dumps(correspondence,indent=2)+'\n')
print('Translated',len(classes),'classes and',len(correspondence),'methods/constructors')
