# BMFont改
This is a tool for creating fonts for CK2 and EU4 which introduced multi-byte DLL.
It was made by forking [BMFont](http://svn.code.sf.net/p/bmfont/code/).

# spec
## simple file name
If the data can fit in one image file, the page size will not be included in the file name.

```
aaa_0.dds ----> aaa.dds
```

## change fnt file format

- fix "info"
 - remove "unicode"
 - remove "outline"
- fix "common"
- remove "page id" line
- fix char[n]
 - remove chnl number
 - remove page number
- remove "chars count" line

## YMD character

Since it is necessary to display it with one byte, I assigned these.

|code point|original mean |change mean|assined character|
|:-:|:-:|:-:|:-:|
|0xE|Shift out|day|日(0x65E5)|
|0xF|Shift in|year|年(0x5E74)|
|0x7|Bel|month|月(0x6708)|

## harf width character

- shrink type 1 cuts the back half of the character.
- shrink type 2 cuts the front half of the character.

|code point| character |shrink type|
|:-:|:-:|:-:|
|0x3002|。|1|
|0x3001|、|1|
|0xff08|（|2|
|0xff09|）|1|
|0x300c|「|2|
|0x300d|」|1|
|0xff5b|｛|2|
|0xff5d|｝|1|

## wave dash issue
Translate 0x301C(wave dash) to 0xFF5E(FULLWIDTH TILDE). See [波ダッシュ#Windowsにおいて起きる問題](https://ja.wikipedia.org/wiki/%E6%B3%A2%E3%83%80%E3%83%83%E3%82%B7%E3%83%A5#Windows%E3%81%AB%E3%81%8A%E3%81%84%E3%81%A6%E8%B5%B7%E3%81%8D%E3%82%8B%E5%95%8F%E9%A1%8C).

## CP1252 Mapping

The fonts of EU4 and CK2 use CP1252.

|code point(unicode)|character|name|maped code point(cp1252)|
|:-:|:-:|:-:|:-:|
|0x20AC|€|EURO SIGN|0x80|
|0x201A|‚|SINGLE LOW-9 QUOTATION MARK|0x82|
|0x0192|ƒ|LATIN SMALL LETTER F WITH HOOK|0x83|
|0x201E|„|DOUBLE LOW-9 QUOTATION MARK|0x84|
|0x2026|…|HORIZONTAL ELLIPSIS|0x85|
|0x2020|†|DAGGER|0x86|
|0x2021|‡|DOUBLE DAGGER|0x87|
|0x20C6|^|MODIFIER LETTER CIRCUMFLEX ACCENT|0x88|
|0x2030|‰|PER MILLE SIGN|0x89|
|0x0160|Š|LATIN CAPITAL LETTER S WITH CARON|0x8A|
|0x2039|‹|SINGLE LEFT-POINTING ANGLE QUOTATION MARK|0x8B|
|0x0152|Œ|LATIN CAPITAL LIGATURE OE|0x8C|
|0x017D|Ž|LATIN CAPITAL LETTER Z WITH CARON|0x8E|
|0x2018|‘|LEFT SINGLE QUOTATION MARK|0x91|
|0x2019|’|RIGHT SINGLE QUOTATION MARK|0x92|
|0x201C|“|LEFT DOUBLE QUOTATION MARK|0x93|
|0x201D|”|RIGHT DOUBLE QUOTATION MARK|0x94|
|0x2022|•|BULLET|0x95|
|0x2013|–|EN DASH|0x96|
|0x2014|—|EM DASH|0x97|
|0x02DC|˜|SMALL TILDE|0x98|
|0x2122|™|TRADE MARK SIGN|0x99|
|0x0161|š|LATIN SMALL LETTER S WITH CARON|0x9A|
|0x203A|›|SINGLE RIGHT-POINTING ANGLE QUOTATION MARK|0x9B|
|0x0153|œ|LATIN SMALL LIGATURE OE|0x9C|
|0x017E|ž|LATIN SMALL LETTER Z WITH CARON|0x9E|
|0x0178|Ÿ|LATIN CAPITAL LETTER Y WITH DIAERESIS|0x9F|

## Prohibited characters area
It can not be used from *0x100* to *0xA00* character. 
The reason is that there are other data in that range of heap memory. Therefore, it shift to the private area(**0xE000**).

