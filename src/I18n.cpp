#include "I18n.h"
#include "Config.h"
#include "Emoji.h"
#include <U8g2_for_Adafruit_GFX.h>
#include "Modules.h" // MM_DUEL: the strings of checkers and sea battle (I18nTable.h)
#include "I18nTable.h"
#include "I18nFonts.h"
#include <initializer_list>
#if !defined(MM_UI_PREVIEW)
#include <esp_spi_flash.h>
#endif

const char* const langCodes[LangCount]={"en","ru"};
const char* const langNames[LangCount]={"English","Русский"};
int langFromCode(const String& code){for(int i=0;i<LangCount;i++)if(code==langCodes[i])return i;return -1;}
bool langAvailable(uint8_t l){return l<LangCount;}
uint8_t langStep(uint8_t l,int dir){for(int i=0;i<LangCount;i++){l=(l+LangCount+(dir<0?-1:1))%LangCount;if(langAvailable(l))break;}return l;}

namespace {
uint8_t lang(){return config.lang<LangCount?config.lang:LangEn;}
// FNV-1a over the parts joined by 0x1f, as tools/i18n.py keys the translations.
uint32_t hashParts(std::initializer_list<const char*> parts){
 uint32_t h=0x811c9dc5;bool first=true;
 for(const char* p:parts){if(!first)h=(h^0x1f)*0x01000193;first=false;while(*p)h=(h^uint8_t(*p++))*0x01000193;}
 return h;
}
// The translation for the interface language, nullptr when there is none.
const char* lookup(uint32_t key){
#if MM_I18N_TABLE
 uint8_t l=lang();if(l<2)return nullptr;
 unsigned lo=0,hi=i18nTable::count;
 while(lo<hi){unsigned mid=(lo+hi)/2;if(i18nTable::keys[mid]<key)lo=mid+1;else hi=mid;}
 if(lo>=i18nTable::count||i18nTable::keys[lo]!=key)return nullptr;
 if(!i18nTable::texts[l-2])return nullptr;
 const char* s=i18nTable::texts[l-2]+i18nTable::offsets[l-2][lo];return *s?s:nullptr;
#else
 (void)key;return nullptr; // no language after en and ru in this image (I18nTable.h)
#endif
}
// CLDR plural categories: 0 zero, 1 one, 2 two, 3 few, 4 many, 5 other.
uint8_t category(uint8_t l,unsigned n){
 unsigned d=n%10,h=n%100;
 switch(l){
 case LangRu:return d==1&&h!=11?1:d>=2&&d<=4&&(h<12||h>14)?3:4;
 default:return n==1?1:5;
 }
}
}

const char* trKey(const char* en,const char* ru,uint32_t key){
 uint8_t l=lang();if(l==LangEn)return en;if(l==LangRu)return ru?ru:en;
 const char* s=lookup(key);return s?s:en;
}

String plural(unsigned n,const char* one,const char* many,const char* ru1,const char* ru2,const char* ru5){
 uint8_t l=lang();String form;
 if(l==LangRu){uint8_t c=category(l,n);form=c==1?ru1:c==3?ru2:ru5;}
 else if(const char* s=l==LangEn?nullptr:lookup(hashParts({one,many,ru1,ru2,ru5}))){
  // six forms separated by 0x1f; a missing one is "other"
  String all(s),forms[6];unsigned at=0;for(int i=0;i<6;i++){int end=all.indexOf('\x1f',at);if(end<0)end=all.length();forms[i]=all.substring(at,end);at=end+1;}
  form=forms[category(l,n)];if(!form.length())form=forms[5];
  // Translations place the number with {n}; a form without it is the whole phrase.
  if(form.length()){form.replace("{n}",String(n));return form;}
 }
 if(!form.length())form=n==1?one:many;
 return String(n)+" "+form;
}

String installLanguage(){
 char b[17]={};
#if defined(MM_UI_PREVIEW)
 return "";
#else
 // The tail of the partition table sector: tools/pages.py writes "MMLANG:<code>" there in partitions-<code>.bin.
 if(spi_flash_read(0x8c00,b,16)!=ESP_OK)return "";
#endif
 b[16]=0;if(strncmp(b,"MMLANG:",7))return "";
 String code(b+7);return langFromCode(code)>=0?code:"";
}

namespace {
uint32_t nextCodepoint(const String& s,unsigned& i){uint8_t c=s[i];unsigned n=c<0x80?1:(c&0xe0)==0xc0?2:(c&0xf0)==0xe0?3:4;uint32_t cp=n==1?c:n==2?c&0x1f:n==3?c&0x0f:c&0x07;for(unsigned k=1;k<n&&i+k<s.length();k++)cp=cp<<6|(uint8_t(s[i+k])&0x3f);i+=n;return cp;}
}

bool fontHasGlyph(const uint8_t* font,uint16_t cp){
 // The lookup of u8g2_font_get_glyph_data (U8g2_for_Adafruit_GFX.cpp), without selecting the font.
 if(!font)return false;auto be16=[](const uint8_t* p){return uint16_t(p[0]<<8|p[1]);};
 const uint8_t* f=font+23;
 if(cp<=255){if(cp>='a')f+=be16(font+19);else if(cp>='A')f+=be16(font+17);for(;;){if(!f[1])return false;if(f[0]==cp)return true;f+=f[1];}}
 f+=be16(font+21);const uint8_t* table=f;uint16_t e;
 do{f+=be16(table);e=be16(table+2);table+=4;}while(e<cp);
 for(;;){e=be16(f);if(!e)return false;if(e==cp)return true;f+=f[2];}
}

uint32_t utf8Next(const String& s,unsigned& i){return nextCodepoint(s,i);}
uint8_t glyphCells(uint32_t cp){
 if(cp>=0x203c){if(emoji::modifier(cp))return 0;if(emoji::flagLetter(cp))return 1;if(emoji::pictograph(cp)||emoji::find(cp)>=0)return 2;} // Emoji.h: two columns
 return (cp>=0x1100&&cp<=0x115f)||(cp>=0x2e80&&cp<=0xa4cf)||(cp>=0xac00&&cp<=0xd7a3)||(cp>=0xf900&&cp<=0xfaff)||(cp>=0xfe30&&cp<=0xfe4f)||(cp>=0xff00&&cp<=0xff60)||(cp>=0xffe0&&cp<=0xffe6)?2:1;
}

const uint8_t* const* fallbackFonts(const uint8_t* font){
 static const uint8_t* list[3];unsigned n=0;
 if(font==u8g2_font_6x13_t_cyrillic){list[n++]=u8g2_font_6x13_tf;list[n++]=mmFontExt13;}
 else if(font==u8g2_font_6x13B_t_cyrillic){list[n++]=u8g2_font_6x13B_tf;list[n++]=mmFontExt13B;}
 else if(font==u8g2_font_5x8_t_cyrillic){list[n++]=u8g2_font_5x8_tf;list[n++]=mmFontExt8;}
 else if(font==u8g2_font_4x6_t_cyrillic){list[n++]=u8g2_font_4x6_tf;list[n++]=mmFontExt6;}
 else if(font==u8g2_font_10x20_t_cyrillic){list[n++]=u8g2_font_10x20_tf;list[n++]=mmFontExt20;}
 else if(font==u8g2_font_6x12_t_cyrillic){list[n++]=u8g2_font_6x12_tf;list[n++]=mmFontExt12;}
 list[n]=nullptr;
 return list;
}
