#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ESP_Mail_Client.h>

//identifiants de la borne wifi
#define WIFI_SSID "Mi"
#define WIFI_PASSWORD "12345678"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 587

/* Identifiants email expéditeur */
#define AUTHOR_EMAIL "mail@gmail.com"
#define AUTHOR_PASSWORD "tonmotdepassemail"

/* Email du destinataire*/
#define RECIPIENT_EMAIL "mail@free.fr"

// parametre machine
#define NOMDUSITE "ECP29"
#define TYPEMAC "Collecteur"
#define NUMEROMACHINE "1"

// Paramètre temporisation
#define TEMPO_ANTI_F_DECT 2000 // en ms
#define TEMPO_CHECK_VIDE 10000 // 86400000 ms pour 24h, prochain check après l'envoi du mail

// definition des broches de l'ESP
#define Switchplein 2

/* Callback function to get the Email sending status*/
//void smtpCallback(SMTP_Status status);

void initWiFi() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connection");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connecté.");
  Serial.println("addresse IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //The ESP8266 tries to reconnect automatically when the connection is lost
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void setup()
{
  Serial.begin(115200);
  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  pinMode(Switchplein, INPUT_PULLUP);
}


void smtpCallback(SMTP_Status status)
{
 // The SMTP Session object used for Email sending
  SMTPSession smtp;
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
    /* Callback function to get the Email sending status */
  }
}
void envoiMail()
{
  // The SMTP Session object used for Email sending
  SMTPSession smtp;
  /** Enable the debug via Serial port
   * none debug or 0
   * basic debug or 1
   */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

 

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "ESP";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Client : " NOMDUSITE " " TYPEMAC " N°" NUMEROMACHINE " plein";
  message.addRecipient("Fab", RECIPIENT_EMAIL);

  /*
  //Send HTML message
  String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Hello World!</h1><p>- Sent from ESP board</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
*/
  // Corps du message
  // Send raw text message
  String textMsg = "Hello World! - Sent from ESP board";
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Set the custom message header */
  // message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}
void loop()
{

  if (digitalRead(Switchplein) == LOW)
  {
    Serial.println("1ere impulsion\n"); // anti appui rapide
    delay(TEMPO_ANTI_F_DECT);
    if (digitalRead(Switchplein) == LOW)
    {
      Serial.println("bouton appuyé\n");
      envoiMail();
      delay(TEMPO_CHECK_VIDE);
      // delay(86400000);//si le mail est parti on attend 24h avant de relancer le test
    }
  }
  else
  {
    Serial.println("bouton relaché\n");
  }
  delay(1000); // pause de 1sec
}
