$cert = New-SelfSignedCertificate -Type Custom -Subject "CN=Microsoft, O=Microsoft, C=US" -KeyUsage DigitalSignature -FriendlyName "1DS SDK Dev TestApp" -CertStoreLocation "Cert:\CurrentUser\My" -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3", "2.5.29.19={text}")
Write-Output "Generated developer certificate:"
Write-Output "$cert"
$password = ConvertTo-SecureString -String password -Force -AsPlainText 
$thumbprint = $cert.Thumbprint
Export-PfxCertificate -cert "Cert:\CurrentUser\My\$thumbprint" -FilePath TestApp.pfx -Password $password
