const fs = require('fs');

function dayNumber() {
  var now = new Date();
  var start = new Date(now.getFullYear(), 0, 0);
  var diff = now - start;
  var oneDay = 1000 * 60 * 60 * 24;
  var dayOfYear = Math.floor(diff / oneDay);
  return dayOfYear;
}

function calculateMinorVersion() {
  var baseYear = 2020; // VER_MINOR starts at 4 in 2020
  var currentYear = new Date().getFullYear();
  return (currentYear - baseYear) + 4; // Increment annually
}

function readAll(filename) {
  var contents = fs.readFileSync(filename, 'utf8');
  return contents;
}

function fileExists(path) {
  return fs.existsSync(path);
}

function generateVersionHpp() {
  var palTxt = "CPP11";
  var verMinor = calculateMinorVersion(); // Calculate minor version dynamically
  var verPatch = dayNumber(); // Day of the year as the patch version

  // Read version tag
  var ver1 = readAll("../Solutions/version.txt");
  // Remove end-of-line
  ver1 = ver1.replace("\n", "");
  // Replace placeholders in version tag
  ver1 = ver1.replace("year", verMinor).replace("day", verPatch);

  // Parse version tag into components
  var ver2 = ver1.split(".").join(",");
  var arr = ver1.split(".");
  var versionHpp = "../lib/include/public/Version.hpp";

  if (fileExists(versionHpp)) {
    var versionHppTxt = readAll(versionHpp);
    if (versionHppTxt.search(ver1) != -1) {
      console.log("Version.hpp " + ver1 + " kept (incremental build)\n");
      return;
    }
    // Delete and recreate
    fs.unlinkSync(versionHpp);
  }

  var templText = readAll("../lib/include/public/Version.hpp.template");
  templText = templText.replace(/\@ull/gi, "@");
  templText = templText.replace(/\@BUILD_VERSION_MAJOR\@/g, arr[0]);
  templText = templText.replace(/\@BUILD_VERSION_MINOR\@/g, arr[1]);
  templText = templText.replace(/\@BUILD_VERSION_PATCH\@/g, arr[2]);
  templText = templText.replace(/\@BUILD_NUMBER\@/g, arr[3]);
  templText = templText.replace(/\@PAL_IMPLEMENTATION_UPPER\@/g, palTxt);

  fs.writeFileSync(versionHpp, templText);
  console.log("Version.hpp " + ver1 + " generated (clean build)\n");
}

generateVersionHpp();

