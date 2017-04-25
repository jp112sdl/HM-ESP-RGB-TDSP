void setStateCCU(String ise_id, String value) {
   if (!Debug) {
    myGLCD.drawBitmap (wifiIconX, wifiIconY, 32, 32, wifiicon32, 90 ,16 ,16); yield();
    http.setTimeout(5000);
    http.begin("http://"+String(ccuip)+"/config/xmlapi/statechange.cgi?ise_id="+ise_id+"&new_value=" + value);
    int httpCode= http.GET();
    Serial.println("setStateCCU with ise_id: "+ise_id+" und Wert "+value+" ergab: "+String(httpCode));

    if(httpCode>0){
//     String payload = http.getString();
    }
    if (httpCode != 200) {
     myGLCD.setColor(255, 0, 0);
     myGLCD.drawLine(319,0,319,240);
    }
    else {
     myGLCD.setColor(0, 255, 0);
     myGLCD.drawLine(319,0,319,240);
    }
    http.end(); 

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect (wifiIconX, wifiIconY, wifiIconX+32, wifiIconY+32);yield();
   }
}

String getStateFromCCU(String ise_id) {
   //Serial.println("getStateFromCCU with ise_id: "+ise_id);
   myGLCD.drawBitmap (wifiIconX, wifiIconY, 32, 32, wifiicon32, 90 ,16 ,16); yield();
   http.setTimeout(5000);
   http.begin("http://"+String(ccuip)+"/config/xmlapi/state.cgi?datapoint_id="+ise_id);
   int httpCode= http.GET();
   String payload = "error";
   if(httpCode>0){
     payload = http.getString();
   }
   if (httpCode != 200) {
    myGLCD.setColor(255, 0, 0);
    myGLCD.drawLine(319,0,319,240);
   }
   else {
     myGLCD.setColor(0, 255, 0);
     myGLCD.drawLine(319,0,319,240);
   }
   http.end();  
   //Serial.println(payload);
   myGLCD.setColor(0, 0, 0);
   myGLCD.fillRect (wifiIconX, wifiIconY, wifiIconX+32, wifiIconY+32);yield();
   return payload;
}
