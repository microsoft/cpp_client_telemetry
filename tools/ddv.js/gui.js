var navOpenInBackgroundTab = 0x1000;

var oIE = new ActiveXObject("InternetExplorer.Application")

oIE.Visible = false;
oIE.Navigate2("http://127.0.0.1:8080/index.html")
//oIE.Navigate2("http://flashdroid.com:8080/index.html")
oIE.AddressBar = false;
oIE.Height = 1000;
oIE.Width  = 1000;
oIE.MenuBar = false;
oIE.StatusBar = false;
oIE.Top = 0;
oIE.Left = 0;
// oIE.TopLevelContainer = true;
oIE.Silent = true;
oIE.Visible = true;

// WScript.sleep(1000);
// oIE.Visible = true;
