# Code Police — spécification du runtime Amiga natif

## 1. Objet

Le port actuel prouve la fidélité du moteur : il reconstruit le graphe Java,
calcule une image ARGB 520×300, puis la réduit vers un écran AGA 320×200 en
16 couleurs. Cette architecture est un oracle de comparaison, pas le runtime
final.

Le runtime final doit être pensé pour la machine cible et pour la vitesse :

- écran AGA 320×200, quatre bitplanes, 16 couleurs maximum ;
- priorité à un 68060, avec un chemin dégradé utilisable sur 68020 ;
- aucune virgule flottante dans la boucle temps réel ;
- aucune décompression PNG, conversion ARGB ou allocation pendant une frame ;
- rendu indexé dès les textures et jusqu’au C2P.

Le port de référence reste conservé séparément afin de mesurer les écarts du
runtime natif. Il ne faut pas ralentir le runtime natif pour préserver une
reproduction impossible à tenir sur l’Amiga.

## 2. Contrat de performance

La cible principale est une présentation stable à 50 Hz PAL, soit 20 ms par
frame. Le budget doit être mesuré séparément :

| Étape | Budget indicatif 68060 | Règle |
|---|---:|---|
| mise à jour script/objets | ≤ 2 ms | entiers, tables précalculées |
| rasterisation indexée | ≤ 12 ms | aucun pixel ARGB intermédiaire |
| C2P 4 plans | ≤ 4 ms | routine assembleur mesurée |
| marge/vblank | ≥ 2 ms | éviter les dépassements systématiques |

Ces chiffres sont des objectifs de conception, pas des mesures déjà acquises.
Une frame qui dépasse le budget doit être comptée, pas masquée. Le runtime doit
afficher son taux de frames et son temps maximum en mode diagnostic.

Sur 68020, la priorité est la lisibilité et le fonctionnement correct ; le
mode peut réduire la fréquence de présentation ou sauter une frame. Le 68060
est la cible de vitesse.

## 3. Architecture mémoire

### Chip RAM

La Chip RAM est réservée aux données que le chipset doit lire :

- quatre bitplanes écran ;
- éventuellement un second jeu de bitplanes pour le double buffering ;
- tables ou buffers explicitement requis par le blitter/C2P.

Le CPU ne doit pas y faire résider les gros buffers de travail. Les bitplanes
doivent être alloués une fois, avec un stride connu, puis réutilisés.

### Fast RAM

Le code, les tables, la scène, les textures indexées, les buffers de travail
et les états des particules vont en Fast RAM. Toute allocation dynamique est
interdite dans la boucle de frame.

Au démarrage :

1. charger et valider le script ;
2. charger les textures ;
3. construire toutes les tables de conversion et de blend ;
4. créer les objets et les contextes ;
5. allouer les buffers écran ;
6. seulement ensuite entrer dans la boucle PAL.

## 4. Formats d’images

### Format runtime obligatoire

Les images runtime sont des bitmaps indexés sur 4 bits : deux pixels par octet.
Chaque texture possède :

- largeur et hauteur en pixels ;
- stride compact en octets ;
- données indexées 0..15 ;
- index de transparence explicite, normalement 0 ;
- palette ou identifiant de palette associé à la scène.

Les PNG et les formats true color restent des formats d’archive ou de
préparation. Ils ne doivent jamais être ouverts par le programme Amiga.

### Résolution

Les textures doivent être préparées pour leur usage réel :

- aucune texture ne doit conserver une résolution supérieure à ce que le
  mapping 320×200 peut effectivement distinguer ;
- les images destinées à l’écran ne dépassent pas 320×200 sans justification ;
- les sprites et flares sont réduits à leur taille maximale d’affichage ;
- les textures répétées peuvent être découpées en tuiles plus petites.

Exception importante : les textures procédurales utilisées par Ground, Tunnel
et Twirl conservent leur grille 256×256 lorsque le moteur original adresse
leurs UV sur huit bits. Les réduire à 200×200 ne représente pas une optimisation
valide : les UV hors de cette zone deviennent des blocs uniformes au lieu de
reboucler dans la texture.

La réduction doit être faite hors ligne, avec une comparaison visuelle et un
rapport de taille. Une réduction aveugle qui change la géométrie apparente ou
les coordonnées UV est refusée.

### Palettes

Le runtime utilise au plus 16 couleurs par scène. Les palettes sont produites
hors ligne à partir des frames de référence, puis embarquées sous forme de
registres AGA 12 bits.

Les fades et les teintes modifient les registres de palette, pas les pixels.
Les opérations additives et de blend utilisent des tables précalculées :

```text
blend[alpha][src_index][dst_index] -> output_index
add[src_index][dst_index]           -> output_index
mix[src_index][dst_index]            -> output_index
```

Les tables sont reconstruites uniquement lors d’un changement de scène ou de
paramètre global, jamais dans la boucle par pixel.

## 5. Arithmétique fixe

La virgule flottante est interdite dans le runtime natif. Les formats proposés
doivent être documentés champ par champ et testés contre les dépassements.

| Usage | Format initial recommandé | Remarque |
|---|---|---|
| coordonnées écran | entier 16 bits | 0..319 et 0..199 |
| position monde | Q16.16 signé | portée et précision à confirmer |
| UV texture | Q8.8 ou Q16.16 | selon taille maximale des textures |
| angle | unsigned 16 bits | tour complet = 65536 |
| sinus/cosinus | table signée Q1.15 | interpolation facultative |
| profondeur/inverse profondeur | Q16.16 | éviter divisions par pixel |
| fade/alpha | unsigned 8 bits | index direct dans les tables |

Les divisions et multiplications coûteuses doivent être déplacées vers :

- des tables précalculées ;
- les changements de part ;
- les calculs par scanline ou par bloc ;
- des routines assembleur lorsque le profilage le justifie.

Chaque conversion depuis l’ancien moteur flottant doit avoir un test de bornes,
un test de déterminisme et une comparaison d’image tolérante adaptée à la
quantification 16 couleurs.

## 6. Pipeline d’une frame

Le pipeline cible est :

```text
tick entier
  -> mise à jour des états fixes
  -> sélection de la part et de sa palette
  -> rendu directement dans un buffer d’indices 4 bits
  -> application des fades par palette
  -> C2P 4 plans vers Chip RAM
  -> attente/présentation au vblank
```

Le buffer de rendu ne doit jamais être un tableau de pixels 32 bits. Les
opérateurs atteignables sont les suivants :

- copie transparente d’indices ;
- blend alpha via table ;
- addition via table ;
- motion blur via table ;
- remplissages de sol, tunnel et twirl par génération d’indices ;
- particules et flares avec texture indexée.

Le rendu est limité aux zones couvertes. Les effets qui ne changent pas une
zone doivent pouvoir être ignorés ou réutiliser un buffer précédent.

## 7. Routines critiques

Le C reste la référence fonctionnelle. Les noyaux chauds peuvent recevoir une
implémentation assembleur 68020/68060 derrière la même interface :

1. C2P 4 plans, largeur 320, stride variable ;
2. copie/blit transparent d’indices ;
3. blend et addition indexés ;
4. remplissage de scanlines ;
5. interpolation UV fixe ;
6. copie de buffers et effacement de frame.

Chaque routine assembleur doit fournir :

- un test vectoriel C indépendant ;
- un chemin C de secours ;
- une mesure de cycles sur la cible ou l’émulateur ;
- des contraintes explicites sur alignement, largeur et stride ;
- une vérification des registres préservés selon l’ABI Amiga.

L’assembleur ne doit pas être introduit pour masquer un mauvais format de
données. Le premier gain attendu vient du passage à l’indexé et au fixe ; le
C2P optimisé vient ensuite.

## 8. Organisation des backends

Deux chemins doivent rester distincts :

### Oracle de préservation

- ARGB 520×300 ;
- virgule flottante possible ;
- comparaison frame par frame avec le moteur C++ ;
- utilisé par les validateurs et les études de fidélité.

### Runtime AGA natif

- 320×200 ;
- indices 4 bits ;
- virgule fixe uniquement ;
- textures CPIL converties en textures indexées 4 bits ;
- tables et buffers préalloués ;
- routines C puis assembleur.

Le choix du backend doit être explicite à la compilation ou au lancement. Le
runtime AGA ne doit pas importer par accident les buffers ou les opérateurs
ARGB de l’oracle.

## 9. Jalons d’implémentation

### N1 — assets natifs

- générer les textures réduites et indexées 4 bits ;
- générer les palettes par part ;
- valider dimensions, transparence et histogrammes ;
- supprimer PNG/stb_image du chemin Amiga.

### N2 — contexte indexé

- ajouter un contexte 4 bits ;
- implémenter clear, copie transparente et C2P ;
- afficher un frame statique sur AGA ;
- mesurer Chip RAM, Fast RAM et temps C2P.

### N3 — primitives fixes

- porter les générateurs de sol, tunnel et twirl ;
- remplacer les calculs UV flottants ;
- ajouter les tables de blend/addition ;
- comparer les captures 16 couleurs à la référence AGA.

### N4 — objets et particules

- porter caméra, sprites, flares et particules en fixe ;
- supprimer toute allocation par frame ;
- valider le déterminisme sur plusieurs cycles.

### N5 — noyaux assembleur

- profiler le C sur 68020 et 68060 ;
- remplacer seulement les noyaux dépassant le budget ;
- valider les implémentations assembleur contre le C.

### N6 — présentation

- palette/fades au vblank ;
- double buffering si nécessaire ;
- compteur de frames et temps maximum ;
- mode de secours si le budget n’est pas tenu.

## 10. Critères d’acceptation

Le runtime natif sera considéré comme prêt lorsque :

- aucun PNG, stb_image, ARGB32 ou flottant n’est utilisé par le chemin Amiga ;
- toutes les textures runtime sont indexées sur 4 bits ;
- la mémoire Chip RAM est explicitement bornée ;
- aucune allocation n’a lieu pendant une frame ;
- le résultat est déterministe sur un cycle complet ;
- les captures AGA sont visuellement cohérentes avec les références ;
- le 68060 tient le budget de frame défini par le profilage ;
- les dépassements sont visibles et mesurés, jamais dissimulés.

Le port actuel et son commit `9dbaeca` constituent le point de départ de cette
seconde phase. Ils ne doivent pas être présentés comme le runtime Amiga final.
