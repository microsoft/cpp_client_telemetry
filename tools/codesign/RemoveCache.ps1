#Script to delete user config from user profile that contains codesign policy data

function RemoveCache
{
$readfile = ([Environment]::GetFolderPath("LocalApplicationData"))
    try {


#delete cache. This gives few warnings but deletes files
     $list = gci $readfile -include "user.config" -recurse -WarningAction SilentlyContinue | select-string -pattern "Codesign.UserSettings.MySettings" , "CODESIGN.PolicyManager.Properties.Setting"

     $list | foreach { rm $_.Path } 

        write-host "Delete Cache Success!"
    }
    Catch
    {
        write-host "Delete Cache Fail"
    }
}

RemoveCache
