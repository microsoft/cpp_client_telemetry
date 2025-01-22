const fs = require('fs');

function dayNumber() {
  const now = new Date();
  const start = new Date(now.getFullYear(), 0, 0);
  const diff = now - start;
  const oneDay = 1000 * 60 * 60 * 24;
  return Math.floor(diff / oneDay);
}

function calculateMinorVersion() {
  const baseYear = 2020; // VER_MINOR starts at 4 in 2020
  const currentYear = new Date().getFullYear();
  return (currentYear - baseYear) + 4; // Increment annually
}

function readAll(filename) {
  return fs.readFileSync(filename, 'utf8');
}

function fileExists(path) {
  return fs.existsSync(path);
}

function generateVersionHpp() {
  const palTxt = "CPP11";
  const verMajor = 3; // Constant major version
  const verMinor = calculateMinorVersion(); // Calculate minor version dynamically
  const verPatch = dayNumber(); // Day of the year as the patch version
  const verBuild = 1; // Constant build number

  // Read version template from Solutions.txt
  let ver1 = readAll("../Solutions/version.txt").replace(/\n/g, "");
  ver1 = ver1.replace("year", verMinor).replace("day", verPatch); // Replace placeholders

  const arr = ver1.split(".");
  const versionHpp = "../lib/include/public/Version.hpp";

  // Check if Version.hpp exists and matches the current version
  if (fileExists(versionHpp)) {
    const versionHppTxt = readAll(versionHpp);
    if (versionHppTxt.includes(ver1)) {
      console.log(`Version.hpp ${ver1} kept (incremental build)\n`);
      return;
    }
    fs.unlinkSync(versionHpp); // Delete if outdated
  }

  // Read template and generate Version.hpp
  const templText = readAll("../lib/include/public/Version.hpp.template")
    .replace(/\@ull/gi, "@")
    .replace(/\@BUILD_VERSION_MAJOR\@/g, arr[0])
    .replace(/\@BUILD_VERSION_MINOR\@/g, arr[1])
    .replace(/\@BUILD_VERSION_PATCH\@/g, arr[2])
    .replace(/\@BUILD_NUMBER\@/g, arr[3])
    .replace(/\@PAL_IMPLEMENTATION_UPPER\@/g, palTxt);

  fs.writeFileSync(versionHpp, templText);
  console.log(`Version.hpp ${ver1} generated (clean build)\n`);
}

generateVersionHpp();

