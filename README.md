# RFID_to_sheet

วาง Code ใน google sheet ที่ต้องการเก็บข้อมูล
```javascript

function doPost(e) {
  var uid = e.parameter.uid;
  
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  sheet.appendRow([new Date(), uid]);
  
  return ContentService.createTextOutput("Success");
}

```

Deploy Google Apps Script และทำการ copy URL มาใส่ 

```c

// Google Apps Script Webhook URL
const char* googleScriptURL = "URL";

```