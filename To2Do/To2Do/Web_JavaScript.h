#ifndef WEB_JAVASCRIPT_H
#define WEB_JAVASCRIPT_H

#include <Arduino.h>
#include "Web_JavaScript_UI.h"
#include "Web_JavaScript_Actions.h"
#include "Web_JavaScript_Core.h"

// Ana JavaScript dosyasi - 3 kutuphaneyi birlestirir
// YUKLENME SIRASI ONEMLI!
// 1. UI - Arayuz render fonksiyonlari
// 2. Actions - Butonlar ve popup'lar
// 3. Core - Veri yonetimi ve server iletisimi
String getJavaScriptCombined() {
  static bool initialized = false;
  static String combinedJS = "";
  
  if (!initialized) {
    combinedJS.reserve(32000); // Memory optimization
    combinedJS += getJavaScriptUI();
    combinedJS += getJavaScriptActions();
    combinedJS += getJavaScriptCore();
    initialized = true;
  }
  
  return combinedJS;
}

const char* getJavaScript() {
  static String js = getJavaScriptCombined();
  return js.c_str();
}

#endif
