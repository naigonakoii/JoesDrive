
  
  
  void recRemote(){
    if (radio.receiveDone()) {

      if(radio.SENDERID == uint8_t(REMOTE_ADDRESS)){
        if (radio.DATALEN != sizeof(recFromRemote)){
          Serial.print("Invalid payload received, not matching Payload struct!");
        }else{
                      Serial.println(recFromRemote.Joy2X); 

          recFromRemote = *(recRemoteData*)radio.DATA;
          SendRemote.sendData();
          delay(5);

       }
      }

      lastRec = millis();
      Send = true;
      recBody();
    }
    
  }
    
  
  void recBody(){
 
      if(Serial1.available() > 0){
        SendBody.receiveData();
      memcpy(bodybuf, &recFromBody, sizeof(recFromBody));
      if(recFromRemote.wireless == 1){
        radio.send(DOME_ADDRESS, bodybuf, sizeof(recFromBody)); 
      }else{
        radio.send(REMOTE_ADDRESS, bodybuf, sizeof(recFromBody));
      }
      delay(5);
    }
    lastSent = millis();
  }

