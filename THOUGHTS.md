# Thoughts and Ideas for future development

## Resources

Many more things than now (Fill, Path) could be "Resources", for example also simple colors.
In the Properties field, one could make these Resources "public", which will insert them in the global "Resources" list.
From there, one could assign Resources to other Objects or Resources which are derived from other Resources, such as "Shades", which is a Color-Resource that takes another Color-Resource and changes parameters (saturation, brightness, hue, ...).
Resources could then be referenced anywhere. In a Color-Resource of a Gradient-Step and so on.

## Decorations

At the moment it is possible to define a "Fill" and "Stroke" Paint for Shapes and Rect Objects.
That is not yet very flexible.
A bit like in Icon-O-Matic, there could be the concept of "Decorations", where certain types of Objects are at first vector shapes.
Perhaps all of these objects inherit from a base-cass which provides an agg::path_storage.
By default, these objects would be fill by a certain "Paint".
On top of that one could add any number of Decorations which have their own Paint and perhaps even their own Transformation relative to the Object.
These decorations are based on the vector shape of their parent object and change it according to their type and parameters.
This type of Decoration could be visualized as a child-object of the parent-object.
This would have the advantage making the Decorations individually accessible within the layer-tree and treat the collapsed composition as a single object at the same time.
It would be even more flexible to be able to detach Decorations and place them anywhere in the layer-tree.
As self-contained Objects, Decorations can function as parent-object of other Decorations.

## FontManager

A GlyphCache entry stores either compressed bitmap or vector path data in a buffer.
WonderBrush actually only ever needs vector paths, the optimization for bitmap-glyphs is rather unnecessary.
It is however not possible to cache complete paths, since agg::path_storage is not thread-safe.
Multiple threads would mess up each the internal iterator mechanism for each other.
Perhaps it would be beneficial to implement a separate iterator for path_storage and attach a separate instance of that in each thread.
Otherwise it cannot work to call FontEngine::init_embedded_adaptors() as is done now, when that means the buffer of a GlyphCache is decompressed into a path_storage object belonging to the FontEngine.
That is even less thread-safe.
Instead there should be some sort of GlyphCacheExtractor class into which the "adapters" of the FontEngine are moved.


## Rendering at scale

Zooming in and zooming out are actually quite different problems.
Zooming in means that a small portion of the document is rendered at original resolution and then scaled up.
Zooming out means that much fewer pixels need to be rendered than would be necessary for the original resolution.
Rendering all pixels of the original resolution and showing only parts of them would result in ugly artifacts.
There is much more benefit in rendering the document at scaled down "original" resolution.

## Bitmaps and Caches in blocks

It seems to make sense to cache the document render output up to a certain object index.
Maybe it makes sense if this cache is divided into evenly sized blocks (portions of the view), which each can have their own object index.
If a block is (partially) needed for rendering, but it is determined to be out-dated, then simply all of the block is rendered so that it is ready in full for the next render.s, so dass der Block beim nächsten Rendering komplett gültig vorliegt.

Where in the object tree should caches be located?

Should there be bitmap caching to disk?

BitmapData
 - contains the actual bitmap data
 - only one global instance of the same data
 - referenceable

BitmapManager
 - Access to BitmapData objects
 - One application-wide instance

BitmapObject
 - new objects create new BitmapData instance and hand over to BitmapManager
 - references a BitmapData
 - actually a Rectangle object with bitmap fill

Applikationsweiter "FillCache", könnte auch BitmapManager beinhalten. Bitmaps werden allerdings immer gemeinsam benutzt.

Fill
FillColor
FillGradient
FillBitmap

Ein "Style" ist nicht wie bei Icon-O-Matic die Füllung, sondern eine Art Container für verschiedene Eigenschaften. Nicht gesetzte Eigenschaften könnten hierarchisch vererbt werden.

Objekte können einen privaten Style haben oder der Style kann öffentlich werden und dann von anderen Objekten benutzt werden. Die Änderungen an dem Style durch irgendeines der Objekte werden in allen Objekten sichtbar.

Wie in Icon-O-Matic könnte man nicht nur globale Styles, sondern auch Füllungen und Pfade verfügbar machen. Allerdings optional, nicht standardmäßig bei neuen Objekten.

Füllungen müssen optional einen Alphakanal (Bitmap) unterstützen. Jedenfalls so, dass man mit dem Fülleimer ins Bild klicken kann und dann nicht nur mit einer Farbe, sondern auch mit allen anderen Fülltypen füllen kann.

Füllfarben sollten auch als "Schattierung" einer anderen Farbe auftreten können. Wenn die Hauptfarbe gelöscht wird, erscheint ein Bestätigungsdialog, ob alle Schattierungen in normale Farben umgewandelt werden sollen. Alternativ könnte man auch eine zusätzliche Eigenschaft im Style verwalten, die dafür sorgt, dass eine Füllung abgedunkelt oder sonst wie verändert wird.


Layouting:
Layouting bedeutet, dass der Objektbaum traversiert wird und Abhängigkeiten final aufgelöst werden. Zum Beispiel könnte eine Objektgröße von der aktuellen Fontgröße abhängen, wobei der Font nur in einem übergeordneten Objekt gesetzt ist. Da die Tools auch fertig "gelayoutete" Objektparameter benötigen werden, ist es sinnvoll, das Layout auf dem Originaldokument durchzuführen. Dazu müssten Veränderungen eines Objektes darauf hinweisen, dass ein Layout das Unterbaums nötig ist. Alternativ könnten absolute Parameter im Originalobjektbaum immer live aufgelöst werden, aber der explizite Layoutvorgang könnte es erleichtern, die Regionen zum Neurendern zu wissen. Prinzipiell ist auch ein Layout der Snapshots notwendig, und zwar asynchron vor jedem Rendervorgang. Hierbei können zum Beispiel der aktuelle Zoom und Scrolloffset in die globalen Matrizen einfließen, welche im Originaldokument außen vor bleiben sollten.


PropertyObject Interface erweitern:
	-> Abfrage nach Anzahl von "PropertyObjects" oder "PropertyCategory" oder
	   "Sub-PropertyObject" (nested PropertyObjects?)
	-> einklappbare Abschnitte für jede einzelne PropertyCategory in ListView
	-> PropertyCategory unterstützt Hinzufügen von Properties.
	-> So wird dann beispielsweise der Style eines Objects im selben
	   PropertyListView anzeig- und editierbar.

Alternative Ansicht für die Layer und Objekte: Die Ansicht zeigt nur die Objekte des aktuellen Layers. Bei Doppelklick auf ein Layer verschiebt sich die Ansicht nach links um ihre Breite (mit schnellem Scrolleffekt) und zeigt jetzt die Objekte des geklickten Layers. Man kann in der Hierarchie nach rechts (rein/tiefer) und links (raus/höher) über Pfeilknöpfe, zusätzlich könnte es ein "Breadcrumb" Control geben, so dass man im aktuellen Pfad springen kann. Dieser könnte von oben nach unten verlaufen. Der Pfad ändert sich durch expliziten Doppelklick auf ein Layer, bleibt aber ansonsten stabil, wenn der Nutzer nur kurz zwischen den Ebenen hin- und herspringt.


Slice-Tool:
Slices sind rechteckige Bereiche auf der Bildfläche, die der Nutzer anlegen, definieren und benennen kann. Beim Exportieren kann man alle Slices exportieren, was jeweils den Bereich unter dem Namen exportiert. Ausgewählt wird der Zielordner und vielleicht ein Dateiname, wo ein Wildcard mit dem Namen des jeweiligen Slices ersetzt wird.
