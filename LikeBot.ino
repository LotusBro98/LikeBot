#include <TroykaLedMatrix.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>



//String user = "alexpan98";
String user = "marg22";

//long user_id = 78646967; //"https://api.vk.com/method/users.get?user_ids=alexpan98&version=5.92&access_token=704a01f4704a01f4704a01f4d5702315c37704a704a01f42c37920924c16a65450ff6ce"


const uint8_t fingerprint[20] = {0x6B, 0xE7, 0x68, 0x25, 0x31, 0xE1, 0x1C, 0x7A, 0x78, 0x24, 0xAD, 0xD3, 0x1D, 0x2C, 0xAE, 0x8C, 0xBF, 0x4A, 0x8F, 0x29};
const String access_token = "704a01f4704a01f4704a01f4d5702315c37704a704a01f42c37920924c16a65450ff6ce";
const String ver = "5.92";


long user_id;

const long MAX_POSTS = 4;
long posts_count;
long post_ids[MAX_POSTS];
long likesForPost[MAX_POSTS] = {};

const int JSON_BUF_SIZE = 4096;

enum check_status {
  CONN_FAILED,
  NEW_LIKE,
  NOTHING
};

const int FRAME_TIME = 300;
const uint8_t heart[][8] {
  {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00011000,
    0b00000000,
    0b00000000,
    0b00000000,
  },
  {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00111100,
    0b00111100,
    0b00011000,
    0b00000000,
    0b00000000,
  },
  {
    0b00000000,
    0b00000000,
    0b01100110,
    0b01111110,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00000000,
  },
  {
    0b00000000,
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
  },
  {
    0b00000000,
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
  },
  {
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
  },
  {
    0b00000000,
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
  },
  {
    0b00000000,
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
  },
  {
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
  },
  {
    0b00000000,
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
  },
  {
    0b00000000,
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
  },
  {
    0b01100110,
    0b11111111,
    0b11111111,
    0b11100111,
    0b11100111,
    0b11111111,
    0b01111110,
    0b00111100,
  },
  {
    0b11111111,
    0b11111111,
    0b11100111,
    0b11000011,
    0b11000011,
    0b11100111,
    0b11111111,
    0b01111110,
  },
  {
    0b11111111,
    0b11100111,
    0b11000011,
    0b10000001,
    0b10000001,
    0b11000011,
    0b11100111,
    0b11111111,
  },
  {
    0b11000011,
    0b10000001,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b10000001,
    0b11000011,
  },
  {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
  },
};

const uint8_t disconnected[][8] {
  {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b01000000,
    0b01100000,
    0b00000000
  },
  {
    0b00000000,
    0b00000000,
    0b00000000,
    0b01100000,
    0b00010000,
    0b01001000,
    0b01101000,
    0b00000000
  },
  {
    0b00000000,
    0b01110000,
    0b00001000,
    0b01100100,
    0b00010010,
    0b01001010,
    0b01101010,
    0b00000000
  },
  {
    0b00000000,
    0b01110000,
    0b00001000,
    0b01100100,
    0b00010010,
    0b00001010,
    0b00001010,
    0b00000000
  },
  {
    0b00000000,
    0b01110000,
    0b00001000,
    0b00000100,
    0b00000010,
    0b00000010,
    0b00000010,
    0b00000000
  },
  {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
  }
};

ESP8266WiFiMulti WiFiMulti;

TroykaLedMatrix matrix;

StaticJsonDocument<JSON_BUF_SIZE> doc;


int askJson(StaticJsonDocument<JSON_BUF_SIZE>& doc, HTTPClient& https, std::unique_ptr<BearSSL::WiFiClientSecure> &client, String& url)
{
  if (https.begin(*client, url)) {
    int httpCode = https.GET();
    if (httpCode > 0) {
      //Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        //String payload = https.getString();
        //Serial.println(payload.length());
        //Serial.println(payload);

        deserializeJson(doc, *client);
        //deserializeJson(doc, payload);
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      return false;
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
    return false;
  }

  return true;
}

int lookUpUser()
{
  if ((WiFiMulti.run() != WL_CONNECTED))
    return CONN_FAILED;

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setFingerprint(fingerprint);

  HTTPClient https;

  if (!askJson(doc, https, client, 
    String("https://api.vk.com/method/users.get?")+
    "user_ids=" + user + 
    "&version=" + ver + 
    "&access_token=" + access_token
  )) return false;
  
  user_id = doc["response"][0]["uid"];
  Serial.println(user_id);
  return true;
}

int lookUpPostIds()
{
  if ((WiFiMulti.run() != WL_CONNECTED))
    return CONN_FAILED;

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setFingerprint(fingerprint);

  HTTPClient https;

  if (!askJson(doc, https, client, 
    String("https://api.vk.com/method/wall.get?")+
    "owner_id=" + user_id + 
    "&version=" + ver + 
    "&count=" + 1 + 
    "&access_token=" + access_token
  )) return false;
  
  posts_count = doc["response"][0];
  if (posts_count > MAX_POSTS)
  posts_count = MAX_POSTS;

  for (long i = 0; i < posts_count; i++)
  {
    if (!askJson(doc, https, client, 
      String("https://api.vk.com/method/wall.get?")+
      "owner_id=" + user_id + 
      "&version=" + ver + 
      "&count=" + 1 +
      "&offset=" + i + 
      "&access_token=" + access_token
    )) return false;

    long id = doc["response"][1]["id"];
    Serial.println(id);

    post_ids[i] = id;
  }
  return true;
}

int checkLikes()
{
  if ((WiFiMulti.run() != WL_CONNECTED))
    return CONN_FAILED;

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setFingerprint(fingerprint);

  HTTPClient https;

  bool liked = false;

  for (long i = 0; i < posts_count; i++)
  {
    long id = post_ids[i];

    if (!askJson(doc, https, client, 
      String("https://api.vk.com/method/likes.getList?")+
      "owner_id=" + user_id +
      "&type=post" + 
      "&item_id=" + id +
      "&version=" + ver + 
      "&access_token=" + access_token
    )) return CONN_FAILED;
    
    long likes = doc["response"]["count"];
    Serial.println(likes);

    if (likes != likesForPost[i])
      liked = true;

    likesForPost[i] = likes;
  }

  if (liked)
    return NEW_LIKE;
  else
    return NOTHING;
}

uint8_t get_bit(uint8_t line, int i)
{
  return (line >> i) & 1;
}

void set_bit(uint8_t &line, int i, uint8_t b)
{
  if (b == 0)
    line &= ~(1 << i);
   else
    line |= 1 << i;
}

void playHeartAnimation()
{
  int FRAMES = sizeof(heart) / 8;
  for (int i = 0; i < FRAMES; i++)
  { 
    uint8_t buf[8];
    for (int j = 0; j < 8; j++)
     for (int k = 0; k < 8; k++)
      set_bit(buf[j], k, get_bit(heart[i][7 - k], j));

    matrix.drawBitmap(buf);
    //matrix.drawBitmap(heart[i]);
    delay(FRAME_TIME);
  }
}

void playDisconnectedAnimation()
{
  int FRAMES = sizeof(disconnected) / 8;
  for (int i = 0; i < FRAMES; i++)
  {
    uint8_t buf[8];
    for (int j = 0; j < 8; j++)
     for (int k = 0; k < 8; k++)
      set_bit(buf[j], k, get_bit(disconnected[i][7 - k], j));
    

    matrix.drawBitmap(buf);
    //matrix.drawBitmap(disconnected[i]);
    delay(FRAME_TIME);
  }
}


void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("WAZZUPSQUADFAM", "88005553535");
  WiFiMulti.addAP("Voodoo Restoration", "");

  matrix.begin();
  matrix.clear();


  while(!lookUpUser())
  {
    playDisconnectedAnimation();
    Serial.println("Failed to connect");
  }

  while(!lookUpPostIds())
  {
    playDisconnectedAnimation();
    Serial.println("Failed to connect");
  }

}


void loop() {
  // wait for WiFi connection

  //int status = checkLikes();

  while (Serial.available() > 0)
  {
    char c = Serial.read();
    if (c == 'e')
      playHeartAnimation();
  }
  delay(100);


  switch (status)
  {
    case NEW_LIKE:
      Serial.println("New like!");
      playHeartAnimation();
      break;
    case CONN_FAILED:
      Serial.println("Failed to connect");
      playDisconnectedAnimation();
      break;
    default:
      matrix.clear();
      break;
  }

  delay(1000);
}
