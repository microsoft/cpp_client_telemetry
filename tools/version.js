var console = {
  log: function (text) {
    try {
        WScript.stdout.Write(text);
        this.log = function (text) { WScript.stdout.Write(text); };
    } catch (er) {
        this.log = function () { };
    }
  }
};

function dayNumber() {
  var now = new Date();
  var start = new Date(now.getFullYear(), 0, 0);
  var diff = now - start;
  var oneDay = 1000 * 60 * 60 * 24;
  var dayOfYear = Math.floor(diff / oneDay);
  return dayOfYear;
}

var fso = WScript.CreateObject("Scripting.Filesystemobject");

function readAll(filename) {
  var file = fso.OpenTextFile(filename, 1);
  var result = file.ReadAll();
  file.close();
  return result;
}

function generateVersionHpp() {

  var palTxt = "WIN32";

  // Read version tag
  var ver1      = readAll("..\\Solutions\\version.txt");
  // Remove end-of-line
  ver1 = ver1.replace("\n", "");
  // Replace 999 by today's dayNumber for nightly builds
  ver1 = ver1.replace("999", dayNumber() );
  // console.log("version.txt => " + ver1 + "\n");
  var ver2 = ver1.split(".").join(",");
  var arr  = ver1.split(".");

  var versionHpp  = "..\\lib\\include\\public\\Version.hpp";
  if (fso.FileExists(versionHpp)) {
    var versionHppTxt = readAll(versionHpp);
    if (versionHppTxt.search(ver1)!=-1) {
      console.log("Version.hpp "+ver1+" kept (incremental build)\n");
      return;
    }
    // Delete and recreate
    fso.DeleteFile(versionHpp);
  }

  var templText = readAll("..\\lib\\include\\public\\Version.hpp.template");
  templText = templText.replace(/\@ull/gi, "@");
  templText = templText.replace(/\@BUILD_VERSION_MAJOR\@/g, arr[0]);
  templText = templText.replace(/\@BUILD_VERSION_MINOR\@/g, arr[1]);
  templText = templText.replace(/\@BUILD_VERSION_PATCH\@/g, arr[2]);
  templText = templText.replace(/\@BUILD_NUMBER\@/g,        arr[3]);
  templText = templText.replace(/\@PAL_IMPLEMENTATION_UPPER\@/g, palTxt);

  var f    = fso.OpenTextFile(versionHpp, 8, true);
  f.WriteLine(templText);
  f.Close();

  console.log("Version.hpp "+ver1+" generated (clean build)\n");
}

function generateVersionTxt() {
  // Read base version and replace '999' with nightly build number
  var version = readAll("..\\Solutions\\version.txt").replace("999", dayNumber());

  // Write version.txt with updated build number
  var versionTxt = "..\\Solutions\\out\\version.txt";
  var f = fso.OpenTextFile(versionTxt, 8, true);
  f.WriteLine(version);
  f.Close();

  console.log("version.txt " + version + " generated\n");
}

generateVersionHpp();
generateVersionTxt();
