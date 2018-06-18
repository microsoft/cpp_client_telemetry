# Define parameters used in script
$server = "codesign.gtm.microsoft.com";
$port = 9556;
$isSSL = $true;
$approver1 = "approver1";
$approver2 = "approver2";
$copsCode = "10006";
$jobPath = "C:\";
$displayName = "Test";
$displayURL = "http://test";
$description = "Test";
$publisherKey = "CertSubject";
$publisherValue = "CN=Microsoft Corporation, O=Microsoft Corporation, L=Redmond, S=Washington, C=US";
$isAllowReturnPartial = $false;

# Function
# Load an assembly from the application directory or from the global assembly cache using a partial name
function AddAssembly ($name) 
{
      return [System.Reflection.Assembly]::LoadWithPartialName($name) | Out-Null;
}

function GetElapsedTime 
{
      param([DateTime]$t1, [DateTime]$t2);
      
      $runtime = $t2 - $t1;
      $elpasedTime = $runtime.Seconds;
      return $elpasedTime;
}

function GetCertificateOperationSet
{
      $cops = "";
      foreach ($key in $job.SelectedCertificateList.Keys)
      {
            $cops += $key + ", ";
      }
      
      return $cops.Substring(0, $cops.Length - 2);
}

function GetPublisherValue
{
      $n = $job.CustomList.Count;
      for ($i = 0; $i -lt $n; $i++) 
      {
            $key = $job.CustomList.Keys[$i];
            if ($key -eq "CertSubject") 
            {
                  return $job.CustomList.Values[$i];
            }
      }
      return $null;
}

function GetPersonList
{
      $flag = $false;
      $n = $job.PersonList.Count;
      $personList = "";
      for ($i = 0; $i -lt $n; $i++)
      {
            if ($job.PersonList.Values[$i].IsSubmitter)
            {
                  $personList += "Submitter: " + $job.PersonList.Values[$i].Alias + ", ";
            }
            elseif ($job.PersonList.Values[$i].IsApprover)
            {
                  if (!$flag) 
                  {
                       $personList += "Approver: " + $job.PersonList.Values[$i].Alias + ", ";
                        $flag = $true;
                  }
                  else
                  {
                        if ($i -eq $n - 1) 
                        {
                              $personList += $job.PersonList.Values[$i].Alias;
                        }
                        else 
                        {
                              $personList += $job.PersonList.Values[$i].Alias + ", ";
                        }
                  }
            }
      }
      
      return $personList;
}

#function GetErrorList
#{
#     $errorMsg = "";
#     foreach ($err in $job.ErrorList)
#     {
#           $errorMsg += $err + ", ";
#     }
#     
#     return $errorMsg.Substring(0, $errorMsg.Length - 2);
#}

$before = Get-Date;

#$o = New-Object -TypeName CODESIGN.Submitter.Job
#Add-Type -TypeDefinition $0
#Add-Type -AssemblyName CODESIGN.Submitter.Job;

AddAssembly "CODESIGN.Submitter";

$job = [CODESIGN.Submitter.Job]::Initialize($server, $port, $isSSL);

$job.AddApprover($approver1);
$job.AddApprover($approver2);

$job.SelectCertificate($copsCode);

$job.AddFile($jobPath, $displayName, $displayURL, [CODESIGN.JavaPermissionsTypeEnum]::None);

$job.Description = $description;
$job.Keywords = $description;
$job.AddCustomFeature($publisherKey, $publisherValue);

$job.IsAllowReturnPartial = $isAllowReturnPartial;

$job.Send();

$after = Get-Date;
$elapsedTime = GetElapsedTime $before $after;
$cops = GetCertificateOperationSet;
$publisher = GetPublisherValue;
#$submitter = $job.PersonList.Values | Where-Object { $_.IsSubmitter -eq $true } | Format-Table -Property Alias;
$personList = GetPersonList;
#$errorList = GetErrorList;

# Job submission info
Write-Host "Job has been submitted with the following info:";
Write-Host "-----------------------------------------------";
Write-Host "Elapsed time             : $elapsedTime seconds";
Write-Host "COPS                     :" $cops;
Write-Host "JobNumber                :" $job.JobNumber;
Write-Host "Number of Files          :" $job.FileCount;
Write-Host "Total job size           :" $job.TotalByteSize "bytes";
Write-Host "HashType                 :" $job.HashType;
Write-Host "PriorityType             :" $job.PriorityType;
Write-Host "SubmissionPath           :" $job.JobSubmissionPath;
Write-Host "CompletionPath           :" $job.JobCompletionPath;
Write-Host "PersonList               :" $personList;
Write-Host "Description              :" $job.Description;
Write-Host "Publisher                :" $publisher;
Write-Host "Submitter ClientVersion  :" $job.ClientVersion;
Write-Host "Submitter CurrentVersion :" $job.CurrentVersion;
Write-Host "Number of Errors         :" $job.ErrorList.Count;
#Write-Host "ErrorList                :" $errorList;
Write-Host "MetricsXML               :" $job.MetricsXML;
Write-Host "RelayServer              :" $job.RelayServer;
Write-Host "RelayPort                :" $job.RelayPort;
Write-Host "RelayMode                :" $job.RelayMode;
Write-Host "MaxRetries               :" $job.MaxRetries;
Write-Host "RetryDelay               :" $job.RetryDelay;
Write-Host "JobRelayInstance         :" $job.JobRelayInstance;
Write-Host "-----------------------------------------------";
Write-Host "Press any key to continue ..."
 
$x = $host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

$jobwatch = New-Object CODESIGN.Submitter.JobWatcher
$jobwatch.Watch($job.jobNumber, $server, $port, $isSSL);
IF ($jobwatch.IsSuccess)
{Write-Host $job.JobNumber               	     :" Has Completed";}
ELSE
{Write-Host $job.JobNumber                           :" Has Failed";}
$afterjw = Get-Date;
$elapsedTimejw = GetElapsedTime $before $afterjw;
Write-Host "Total Elapsed time       : $elapsedTimejw seconds"

foreach ($ff in $jobwatch.FailedFileList.Values)
{
Write-Host "Sign Failure: Individual file could not be signed:" $ff.FileFullPath
}

foreach ($jelv in $jobwatch.ErrorList.Values)
{
Write-Host "Job Error List:" $jelv.Number + $jelv.Description + $jelv.Explanation
}
$jobwatch = $null;
$job = $null;

# SIG # Begin signature block
# MIIj5QYJKoZIhvcNAQcCoIIj1jCCI9ICAQExDzANBglghkgBZQMEAgEFADB5Bgor
# BgEEAYI3AgEEoGswaTA0BgorBgEEAYI3AgEeMCYCAwEAAAQQH8w7YFlLCE63JNLG
# KX7zUQIBAAIBAAIBAAIBAAIBADAxMA0GCWCGSAFlAwQCAQUABCCXI+75MKxUp9GW
# 1P8H8whzjGmESarlawwYVT6wC9Di5KCCDZIwggYQMIID+KADAgECAhMzAAAAGne7
# dLMH0Ra4AAAAAAAaMA0GCSqGSIb3DQEBCwUAMH4xCzAJBgNVBAYTAlVTMRMwEQYD
# VQQIEwpXYXNoaW5ndG9uMRAwDgYDVQQHEwdSZWRtb25kMR4wHAYDVQQKExVNaWNy
# b3NvZnQgQ29ycG9yYXRpb24xKDAmBgNVBAMTH01pY3Jvc29mdCBDb2RlIFNpZ25p
# bmcgUENBIDIwMTEwHhcNMTMwOTI0MTc0MTQxWhcNMTQxMjI0MTc0MTQxWjCBgzEL
# MAkGA1UEBhMCVVMxEzARBgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcTB1JlZG1v
# bmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlvbjENMAsGA1UECxMETU9Q
# UjEeMBwGA1UEAxMVTWljcm9zb2Z0IENvcnBvcmF0aW9uMIIBIjANBgkqhkiG9w0B
# AQEFAAOCAQ8AMIIBCgKCAQEA5XwvLQyp7HqoNOBMP39JDg22Fa0ZE95SiiaZFXGp
# YicHN6WDMIJibAuj/QYNFxQG5uCtzJWWCiBaopbh4FcwPF1im8VdiQzQNN/Z2Po1
# 7xEji8D560r0OdovcRDrEbMsN6Nw6IYXPu8qRtCOx7lIAKE38cfI59Iea0oq8sZM
# HXCffMNoQo487YEaUuM+MpQ9fhjxm+RLXBHk1sOFHmwDMHO8yagBfZ2t0fVz8Fsa
# ey8fizK+s461O9n3//NfsxN8E5NXuKBeNZiDoTQ08sUEn7n+RhcMkd/vD1X27Mw5
# yWFloSnuvhE3G7duQlXJzDXRUrMDcJyYNJ4pNqkXGV8LvwIDAQABo4IBfzCCAXsw
# HwYDVR0lBBgwFgYIKwYBBQUHAwMGCisGAQQBgjdMCAEwHQYDVR0OBBYEFCQrPcqQ
# nJ4odXI8zwyzPeasJFZZMFEGA1UdEQRKMEikRjBEMQ0wCwYDVQQLEwRNT1BSMTMw
# MQYDVQQFEyozMTY0MisyODYwYjUyZS1jNGEzLTQ1NGQtYmMxZS0zMmM1YWRkMTdl
# OTAwHwYDVR0jBBgwFoAUSG5k5VAF04KqFzc3IrVtqMp1ApUwVAYDVR0fBE0wSzBJ
# oEegRYZDaHR0cDovL3d3dy5taWNyb3NvZnQuY29tL3BraW9wcy9jcmwvTWljQ29k
# U2lnUENBMjAxMV8yMDExLTA3LTA4LmNybDBhBggrBgEFBQcBAQRVMFMwUQYIKwYB
# BQUHMAKGRWh0dHA6Ly93d3cubWljcm9zb2Z0LmNvbS9wa2lvcHMvY2VydHMvTWlj
# Q29kU2lnUENBMjAxMV8yMDExLTA3LTA4LmNydDAMBgNVHRMBAf8EAjAAMA0GCSqG
# SIb3DQEBCwUAA4ICAQARcmAL8qwmtgQFZtw4WNfUbNojqmAtQSi4az+Ucv8+YlFl
# WRcxUQs/1AkOODaDKed4mFRtpBbBLkTAyFGgPtJQRYXmwrx5bltbNOEW1DE5xSNZ
# twfnTLGJT4Bix1egSX6ewxjAIkbosG3mIMYpHwiahG3KgA6Dvb0W3n++CPPNgHmR
# 0/jc6ENM9mYCEmr0gI5LBtFtiAxb0YEc7fSyEcWQL1SbjWAHJCGBbTTLkp2OqkAT
# QqwgP+D2EGXAW8qAqNqO74Zal6T1EUuGhW9iq4ouoE8oDRL5xc2RRwMKxDbJY9fd
# 94V6WP1bh+xmwZj8je5YeqKe0R17iUmOHvs+w7KK7sRnSKV1sf3ObKazoOAUV4hv
# fEjxSIxPkh6rbCRkDLqxrojpXY0vPwtRAlxqrKwZTNRMel+5G7VONa5rsb/YWiit
# SMoVU7iLtCyM6sSkE+SXGVEm8SAffqTD467He4n5Y6G2qAZcQt34ZHfvBjGlH+kR
# X8ukoBVonRjxKHt6E65yRiC/In0ITCDSMJYxDZbUJ/RGRWAtkczX8LpUeUZNnjDo
# 2yArMLZyFbFBp5TgopQlB/z4ee0mwvBmPY+D/vrjlEEznXdGvSWsAqSrSOqHbVtA
# vSbv9RgXORz8yd3Z7L7TpAVkgLjXu3q1IcYII3uBspyMPFz7C+JT4sHpug9fUDCC
# B3owggVioAMCAQICCmEOkNIAAAAAAAMwDQYJKoZIhvcNAQELBQAwgYgxCzAJBgNV
# BAYTAlVTMRMwEQYDVQQIEwpXYXNoaW5ndG9uMRAwDgYDVQQHEwdSZWRtb25kMR4w
# HAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xMjAwBgNVBAMTKU1pY3Jvc29m
# dCBSb290IENlcnRpZmljYXRlIEF1dGhvcml0eSAyMDExMB4XDTExMDcwODIwNTkw
# OVoXDTI2MDcwODIxMDkwOVowfjELMAkGA1UEBhMCVVMxEzARBgNVBAgTCldhc2hp
# bmd0b24xEDAOBgNVBAcTB1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jw
# b3JhdGlvbjEoMCYGA1UEAxMfTWljcm9zb2Z0IENvZGUgU2lnbmluZyBQQ0EgMjAx
# MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAKvw+nIQHC6t2G6qghBN
# NLrytlghn0IbKmvpWlCquAY4GgRJun/DDB7dN2vGEtgL8DjCmQawyDnVARQxQtOJ
# DXlkh36UYCRsr55JnOloXtLfm1OyCizDr9mpK656Ca/XllnKYBoF6WZ26DJSJhIv
# 56sIUM+zRLdd2MQuA3WraPPLbfM6XKEW9Ea64DhkrG5kNXimoGMPLdNAk/jj3gcN
# 1Vx5pUkp5w2+oBN3vpQ97/vjK1oQH01WKKJ6cuASOrdJXtjt7UORg9l7snuGG9k+
# sYxd6IlPhBryoS9Z5JA7La4zWMW3Pv4y07MDPbGyr5I4ftKdgCz1TlaRITUlwzlu
# ZH9TupwPrRkjhMv0ugOGjfdf8NBSv4yUh7zAIXQlXxgotswnKDglmDlKNs98sZKu
# HCOnqWbsYR9q4ShJnV+I4iVd0yFLPlLEtVc/JAPw0XpbL9Uj43BdD1FGd7P4AOG8
# rAKCX9vAFbO9G9RVS+c5oQ/pI0m8GLhEfEXkwcNyeuBy5yTfv0aZxe/CHFfbg43s
# TUkwp6uO3+xbn6/83bBm4sGXgXvt1u1L50kppxMopqd9Z4DmimJ4X7IvhNdXnFy/
# dygo8e1twyiPLI9AN0/B4YVEicQJTMXUpUMvdJX3bvh4IFgsE11glZo+TzOE2rCI
# F96eTvSWsLxGoGyY0uDWiIwLAgMBAAGjggHtMIIB6TAQBgkrBgEEAYI3FQEEAwIB
# ADAdBgNVHQ4EFgQUSG5k5VAF04KqFzc3IrVtqMp1ApUwGQYJKwYBBAGCNxQCBAwe
# CgBTAHUAYgBDAEEwCwYDVR0PBAQDAgGGMA8GA1UdEwEB/wQFMAMBAf8wHwYDVR0j
# BBgwFoAUci06AjGQQ7kUBU7h6qfHMdEjiTQwWgYDVR0fBFMwUTBPoE2gS4ZJaHR0
# cDovL2NybC5taWNyb3NvZnQuY29tL3BraS9jcmwvcHJvZHVjdHMvTWljUm9vQ2Vy
# QXV0MjAxMV8yMDExXzAzXzIyLmNybDBeBggrBgEFBQcBAQRSMFAwTgYIKwYBBQUH
# MAKGQmh0dHA6Ly93d3cubWljcm9zb2Z0LmNvbS9wa2kvY2VydHMvTWljUm9vQ2Vy
# QXV0MjAxMV8yMDExXzAzXzIyLmNydDCBnwYDVR0gBIGXMIGUMIGRBgkrBgEEAYI3
# LgMwgYMwPwYIKwYBBQUHAgEWM2h0dHA6Ly93d3cubWljcm9zb2Z0LmNvbS9wa2lv
# cHMvZG9jcy9wcmltYXJ5Y3BzLmh0bTBABggrBgEFBQcCAjA0HjIgHQBMAGUAZwBh
# AGwAXwBwAG8AbABpAGMAeQBfAHMAdABhAHQAZQBtAGUAbgB0AC4gHTANBgkqhkiG
# 9w0BAQsFAAOCAgEAZ/KGpZjgVHkaLtPYdGcimwuWEeFjkplCln3SeQyQwWVfLiw+
# +MNy0W2D/r4/6ArKO79HqaPzadtjvyI1pZddZYSQfYtGUFXYDJJ80hpLHPM8QotS
# 0LD9a+M+By4pm+Y9G6XUtR13lDni6WTJRD14eiPzE32mkHSDjfTLJgJGKsKKELuk
# qQUMm+1o+mgulaAqPyprWEljHwlpblqYluSD9MCP80Yr3vw70L01724lruWvJ+3Q
# 3fMOr5kol5hNDj0L8giJ1h/DMhji8MUtzluetEk5CsYKwsatruWy2dsViFFFWDgy
# cScaf7H0J/jeLDogaZiyWYlobm+nt3TDQAUGpgEqKD6CPxNNZgvAs0314Y9/HG8V
# fUWnduVAKmWjw11SYobDHWM2l4bf2vP48hahmifhzaWX0O5dY0HjWwechz4GdwbR
# BrF1HxS+YWG18NzGGwS+30HHDiju3mUv7Jf2oVyW2ADWoUa9WfOXpQlLSBCZgB/Q
# ACnFsZulP0V3HjXG0qKin3p6IvpIlR+r+0cjgPWe+L9rt0uX4ut1eBrs6jeZeRhL
# /9azI2h15q/6/IvrC4DqaTuv/DDtBEyO3991bWORPdGdVk5Pv4BXIqF4ETIheu9B
# CrE/+6jMpF3BoYibV3FWTkhFwELJm3ZbCoBIa/15n8G9bW1qyVJzEw16UM0xghWp
# MIIVpQIBATCBlTB+MQswCQYDVQQGEwJVUzETMBEGA1UECBMKV2FzaGluZ3RvbjEQ
# MA4GA1UEBxMHUmVkbW9uZDEeMBwGA1UEChMVTWljcm9zb2Z0IENvcnBvcmF0aW9u
# MSgwJgYDVQQDEx9NaWNyb3NvZnQgQ29kZSBTaWduaW5nIFBDQSAyMDExAhMzAAAA
# Gne7dLMH0Ra4AAAAAAAaMA0GCWCGSAFlAwQCAQUAoIGYMBkGCSqGSIb3DQEJAzEM
# BgorBgEEAYI3AgEEMBwGCisGAQQBgjcCAQsxDjAMBgorBgEEAYI3AgEVMCwGCisG
# AQQBgjcCAQwxHjAcoAqACABUAGUAcwB0oQ6ADGh0dHA6Ly9UZXN0IDAvBgkqhkiG
# 9w0BCQQxIgQgRuiS1n5qU2f3/3YiU7cmpQQi8VVqRuNfGCXa9kZtRGAwDQYJKoZI
# hvcNAQEBBQAEggEAFRtBad0bRjsHUUs74R4FUdDuLCNo/6sxTQOIIIKUc2iQoOIB
# aSwbogEv5qt0QGJf/Wv7uX1aC46E5Vys4bKi5QRaPKeUd8gVlnUVC5gS5KU4qMwj
# uxx3JAG0DLAD7XpdtHikN6Y0tK6sFf9VKaOkOYPvrvwv3lOtdhwMd7Bwv3NYIXBd
# yfUL/Ciajlx9dYFrXxiIOhAbwNbEkLx3JQw2UF3Py0zlVIjmtTfOpjBI3EXnrJRF
# I4Yu/7ENtbdn3dlBSL7mTcGBABu7tk+YkGAMklisyogd4cRCnWs71jvJqAK5XLn+
# 4qzPQM8Gtah8NWL5IUPczT+Qe5d/Csx1L+P3bqGCE0kwghNFBgorBgEEAYI3AwMB
# MYITNTCCEzEGCSqGSIb3DQEHAqCCEyIwghMeAgEDMQ8wDQYJYIZIAWUDBAIBBQAw
# ggE8BgsqhkiG9w0BCRABBKCCASsEggEnMIIBIwIBAQYKKwYBBAGEWQoDATAxMA0G
# CWCGSAFlAwQCAQUABCBz7AoAIRbIrzYyyqWiHzf96LDj5PxDkHpIU1mDbXGZ9QIG
# Uw9sa8bSGBIyMDE0MDMxODE1MjczMi4wNFowBwIBAYACAfSggbmkgbYwgbMxCzAJ
# BgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNoaW5ndG9uMRAwDgYDVQQHEwdSZWRtb25k
# MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xDTALBgNVBAsTBE1PUFIx
# JzAlBgNVBAsTHm5DaXBoZXIgRFNFIEVTTjpCOEVDLTMwQTQtNzE0NDElMCMGA1UE
# AxMcTWljcm9zb2Z0IFRpbWUtU3RhbXAgU2VydmljZaCCDs0wggZxMIIEWaADAgEC
# AgphCYEqAAAAAAACMA0GCSqGSIb3DQEBCwUAMIGIMQswCQYDVQQGEwJVUzETMBEG
# A1UECBMKV2FzaGluZ3RvbjEQMA4GA1UEBxMHUmVkbW9uZDEeMBwGA1UEChMVTWlj
# cm9zb2Z0IENvcnBvcmF0aW9uMTIwMAYDVQQDEylNaWNyb3NvZnQgUm9vdCBDZXJ0
# aWZpY2F0ZSBBdXRob3JpdHkgMjAxMDAeFw0xMDA3MDEyMTM2NTVaFw0yNTA3MDEy
# MTQ2NTVaMHwxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNoaW5ndG9uMRAwDgYD
# VQQHEwdSZWRtb25kMR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xJjAk
# BgNVBAMTHU1pY3Jvc29mdCBUaW1lLVN0YW1wIFBDQSAyMDEwMIIBIjANBgkqhkiG
# 9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqR0NvHcRijog7PwTl/X6f2mUa3RUENWlCgCC
# hfvtfGhLLF/Fw+Vhwna3PmYrW/AVUycEMR9BGxqVHc4JE458YTBZsTBED/FgiIRU
# QwzXTbg4CLNC3ZOs1nMwVyaCo0UN0Or1R4HNvyRgMlhgRvJYR4YyhB50YWeRX4FU
# sc+TTJLBxKZd0WETbijGGvmGgLvfYfxGwScdJGcSchohiq9LZIlQYrFd/XcfPfBX
# day9ikJNQFHRD5wGPmd/9WbAA5ZEfu/QS/1u5ZrKsajyeioKMfDaTgaRtogINeh4
# HLDpmc085y9Euqf03GS9pAHBIAmTeM38vMDJRF1eFpwBBU8iTQIDAQABo4IB5jCC
# AeIwEAYJKwYBBAGCNxUBBAMCAQAwHQYDVR0OBBYEFNVjOlyKMZDzQ3t8RhvFM2ha
# hW1VMBkGCSsGAQQBgjcUAgQMHgoAUwB1AGIAQwBBMAsGA1UdDwQEAwIBhjAPBgNV
# HRMBAf8EBTADAQH/MB8GA1UdIwQYMBaAFNX2VsuP6KJcYmjRPZSQW9fOmhjEMFYG
# A1UdHwRPME0wS6BJoEeGRWh0dHA6Ly9jcmwubWljcm9zb2Z0LmNvbS9wa2kvY3Js
# L3Byb2R1Y3RzL01pY1Jvb0NlckF1dF8yMDEwLTA2LTIzLmNybDBaBggrBgEFBQcB
# AQROMEwwSgYIKwYBBQUHMAKGPmh0dHA6Ly93d3cubWljcm9zb2Z0LmNvbS9wa2kv
# Y2VydHMvTWljUm9vQ2VyQXV0XzIwMTAtMDYtMjMuY3J0MIGgBgNVHSABAf8EgZUw
# gZIwgY8GCSsGAQQBgjcuAzCBgTA9BggrBgEFBQcCARYxaHR0cDovL3d3dy5taWNy
# b3NvZnQuY29tL1BLSS9kb2NzL0NQUy9kZWZhdWx0Lmh0bTBABggrBgEFBQcCAjA0
# HjIgHQBMAGUAZwBhAGwAXwBQAG8AbABpAGMAeQBfAFMAdABhAHQAZQBtAGUAbgB0
# AC4gHTANBgkqhkiG9w0BAQsFAAOCAgEAB+aIUQ3ixuCYP4FxAz2do6Ehb7Prpsz1
# Mb7PBeKp/vpXbRkws8LFZslq3/Xn8Hi9x6ieJeP5vO1rVFcIK1GCRBL7uVOMzPRg
# Eop2zEBAQZvcXBf/XPleFzWYJFZLdO9CEMivv3/Gf/I3fVo/HPKZeUqRUgCvOA8X
# 9S95gWXZqbVr5MfO9sp6AG9LMEQkIjzP7QOllo9ZKby2/QThcJ8ySif9Va8v/rbl
# jjO7Yl+a21dA6fHOmWaQjP9qYn/dxUoLkSbiOewZSnFjnXshbcOco6I8+n99lmqQ
# eKZt0uGc+R38ONiU9MalCpaGpL2eGq4EQoO4tYCbIjggtSXlZOz39L9+Y1klD3ou
# OVd2onGqBooPiRa6YacRy5rYDkeagMXQzafQ732D8OE7cQnfXXSYIghh2rBQHm+9
# 8eEA3+cxB6STOvdlR3jo+KhIq/fecn5ha293qYHLpwmsObvsxsvYgrRyzR30uIUB
# HoD7G4kqVDmyW9rIDVWZeodzOwjmmC3qjeAzLhIp9cAvVCch98isTtoouLGp25ay
# p0Kiyc8ZQU3ghvkqmqMRZjDTu3QyS99je/WZii8bxyGvWbWu3EQ8l1Bx16HSxVXj
# ad5XwdHeMMD9zOZN+w2/XU/pnR4ZOC+8z1gFLu8NoFA12u8JJxzVs341Hgi62jbb
# 01+P3nSISRIwggTaMIIDwqADAgECAhMzAAAAKp9LI1/PsPCdAAAAAAAqMA0GCSqG
# SIb3DQEBCwUAMHwxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNoaW5ndG9uMRAw
# DgYDVQQHEwdSZWRtb25kMR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24x
# JjAkBgNVBAMTHU1pY3Jvc29mdCBUaW1lLVN0YW1wIFBDQSAyMDEwMB4XDTEzMDMy
# NzIwMTMxNFoXDTE0MDYyNzIwMTMxNFowgbMxCzAJBgNVBAYTAlVTMRMwEQYDVQQI
# EwpXYXNoaW5ndG9uMRAwDgYDVQQHEwdSZWRtb25kMR4wHAYDVQQKExVNaWNyb3Nv
# ZnQgQ29ycG9yYXRpb24xDTALBgNVBAsTBE1PUFIxJzAlBgNVBAsTHm5DaXBoZXIg
# RFNFIEVTTjpCOEVDLTMwQTQtNzE0NDElMCMGA1UEAxMcTWljcm9zb2Z0IFRpbWUt
# U3RhbXAgU2VydmljZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJWk
# WWZ2qHIlAdIBfg86T1PHeSmGw1r5l8NeG86x6V14jqoHTAf++cV355DAGBRpV1YN
# Jgm7JvG9g9y0SMcKpln9xhO+g7LuWZdjQMDv1mg0g6M3ScDXsaV0T7wCYCFUhqSX
# rWOYICzHPqrXQNNhBH3FPV3r5SvwDF+7PGStwes0svDe61wOCEZPr+puYJGGFIzU
# ZeB4M/mf/cdBlk7T6yh7O4E15YytLOc0l9LUH9LVu/MNpuTuL6RWpQbh0+EmJTCE
# ukldx85sU+Gj8T1KdxCuB8EaFelVXPBANlqOKE7JEvqNsAwlGS+05BNGA0uDIIlb
# 4r4Dongv+0TW/6C/t80CAwEAAaOCARswggEXMB0GA1UdDgQWBBRrmX5GU3jxzASH
# yk1G4T3iyZz1gTAfBgNVHSMEGDAWgBTVYzpcijGQ80N7fEYbxTNoWoVtVTBWBgNV
# HR8ETzBNMEugSaBHhkVodHRwOi8vY3JsLm1pY3Jvc29mdC5jb20vcGtpL2NybC9w
# cm9kdWN0cy9NaWNUaW1TdGFQQ0FfMjAxMC0wNy0wMS5jcmwwWgYIKwYBBQUHAQEE
# TjBMMEoGCCsGAQUFBzAChj5odHRwOi8vd3d3Lm1pY3Jvc29mdC5jb20vcGtpL2Nl
# cnRzL01pY1RpbVN0YVBDQV8yMDEwLTA3LTAxLmNydDAMBgNVHRMBAf8EAjAAMBMG
# A1UdJQQMMAoGCCsGAQUFBwMIMA0GCSqGSIb3DQEBCwUAA4IBAQAnZLSQsWLvIUIy
# +rqs9JEFh1i2TGGZj/aDhOHmnWlwkR9rtvcFIHNPXhTrfOlxiXLyX4h6exQuLr2Q
# uYy6RuGRKvOa545KnNNxZlkSPJ50f1vvSuZyUoldRHJTznF9S8RCoEqjS+WF6dem
# DfhGwfsz/x+OPVxCIfXnE3M4nEiz4ITVnxQ1E5m8k0kIcMW+uh7C+edZgI/aPDz7
# S+VNvWd5zLywDQnOQAaQgpXt3hHtbZrCBH8NL6KJ5oX4AxzNnxAyhXzMNHyMk62V
# mOAciHTvQ5Mhvs7+BIGcn7aoE1D3dF1QIYfbsMs/DkjbKYWuy9wuZG9iFVWde8bE
# EhUiV4l6oYIDdjCCAl4CAQEwgeOhgbmkgbYwgbMxCzAJBgNVBAYTAlVTMRMwEQYD
# VQQIEwpXYXNoaW5ndG9uMRAwDgYDVQQHEwdSZWRtb25kMR4wHAYDVQQKExVNaWNy
# b3NvZnQgQ29ycG9yYXRpb24xDTALBgNVBAsTBE1PUFIxJzAlBgNVBAsTHm5DaXBo
# ZXIgRFNFIEVTTjpCOEVDLTMwQTQtNzE0NDElMCMGA1UEAxMcTWljcm9zb2Z0IFRp
# bWUtU3RhbXAgU2VydmljZaIlCgEBMAkGBSsOAwIaBQADFQAkfYJ44IESW3V+5Lg8
# GfLZezz94qCBwjCBv6SBvDCBuTELMAkGA1UEBhMCVVMxEzARBgNVBAgTCldhc2hp
# bmd0b24xEDAOBgNVBAcTB1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jw
# b3JhdGlvbjENMAsGA1UECxMETU9QUjEnMCUGA1UECxMebkNpcGhlciBOVFMgRVNO
# OkIwMjctQzZGOC0xRDg4MSswKQYDVQQDEyJNaWNyb3NvZnQgVGltZSBTb3VyY2Ug
# TWFzdGVyIENsb2NrMA0GCSqGSIb3DQEBBQUAAgUA1tIQhDAiGA8yMDE0MDMxODAw
# MjMzMloYDzIwMTQwMzE5MDAyMzMyWjB0MDoGCisGAQQBhFkKBAExLDAqMAoCBQDW
# 0hCEAgEAMAcCAQACAgLpMAcCAQACAhZoMAoCBQDW02IEAgEAMDYGCisGAQQBhFkK
# BAIxKDAmMAwGCisGAQQBhFkKAwGgCjAIAgEAAgMW42ChCjAIAgEAAgMHoSAwDQYJ
# KoZIhvcNAQEFBQADggEBAItUC+RZgpR/w7XxkQ0/nhoYMpM50LEyoFwRWtyWIpae
# rKFZmgECU8PdCyT+ca/Zt/H0JI8zlecsqqNY8wjYatEQ7DcCStvalhQWolw087l8
# OVoxaaBqqWv3jw9BAnMs2ZSM+DvTsXRTTtL+6xiRIPTazuSKK08saYIVnCd/sTyS
# 8oDJqEj13D6BVRNM7cLuCf8u3MCl53FwMck/Dl52Kt9c700/fI45acHfBoqueVAA
# gT0+bNnqHjMPotdvhnZ1oL/zYma6Zf3eruZcoyQuZKFCAss4oxjy87Ltw/3QqAmK
# RsG38Bx6gROQfetA+Uk6nKdGTY3RdgeDeXRM8s6LL78xggL1MIIC8QIBATCBkzB8
# MQswCQYDVQQGEwJVUzETMBEGA1UECBMKV2FzaGluZ3RvbjEQMA4GA1UEBxMHUmVk
# bW9uZDEeMBwGA1UEChMVTWljcm9zb2Z0IENvcnBvcmF0aW9uMSYwJAYDVQQDEx1N
# aWNyb3NvZnQgVGltZS1TdGFtcCBQQ0EgMjAxMAITMwAAACqfSyNfz7DwnQAAAAAA
# KjANBglghkgBZQMEAgEFAKCCATIwGgYJKoZIhvcNAQkDMQ0GCyqGSIb3DQEJEAEE
# MC8GCSqGSIb3DQEJBDEiBCD7SujL+3Xdeiki9ski7+VGeLzQzWBeWYbvvGo9fUcQ
# YzCB4gYLKoZIhvcNAQkQAgwxgdIwgc8wgcwwgbEEFCR9gnjggRJbdX7kuDwZ8tl7
# PP3iMIGYMIGApH4wfDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCldhc2hpbmd0b24x
# EDAOBgNVBAcTB1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlv
# bjEmMCQGA1UEAxMdTWljcm9zb2Z0IFRpbWUtU3RhbXAgUENBIDIwMTACEzMAAAAq
# n0sjX8+w8J0AAAAAACowFgQUYSBPAcLQW/Uq+fhLuWC15WzuzycwDQYJKoZIhvcN
# AQELBQAEggEAVkrld7IElM5DhDeeLQeV69zJajPrfDsrUie5FtkIMQXJuKKTczQT
# YYksxcmU63nQPAnd3PdXUIvBqMr1oH9EL54iNyA2xpZLs3Ami1UFiQUfiukydkgi
# 4ivFktBqcpSM4G79BpwE2rrEj++lXYYpbkAMcD1kOmLtFNevEye4ZWne+lLjf2RN
# SU8DyknPh2trex4RTrPU+gxuWUWuvQNMU5haUj8OqoEJVFRe75HHHn3hDZUa/4hM
# 4X4xifmm+iFzG8pEFTJESn3inbvhA2DZmfd+WsZrEnRRhGS7int/jdZiSeD5sARL
# jReWPPEliQgnfU60W7wuVhueFagkXipZQA==
# SIG # End signature block
