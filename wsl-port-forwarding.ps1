If (-NOT ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {   
  $arguments = "& '" + $myinvocation.mycommand.definition + "'"
  Start-Process powershell -Verb runAs -ArgumentList $arguments
  Break
}
$ifconfig = wsl.exe ifconfig eth0
$remoteport = $ifconfig| Select-String -Pattern "inet "
$found = $remoteport -match '\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}';

if ($found) {
  $remoteport = $matches[0];
}
else {
  Write-Output "IP address could not be found";
  exit;
}




Invoke-Expression "netsh interface portproxy reset";

$portl = $args
$portl+= "4500"
$portl+= "4510"
$portl+= "4520"
$portl+= "4530"
$portl+= "4540"
$portl+= "4550"
$portl+= "4560"
$portl+= "4570"
$portl+= "4580"
$portl+= "4590"
$portl+= "4600"
$portl+= "4610"
$portl+= "4620"
$portl+= "4630"
$portl+= "4640"
$portl+= "4650"
$portl+= "4660"
$portl+= "4670"
$portl+= "4680"
$portl+= "4690"
$portl+= "4700"

for ($i = 0; $i -lt $portl.length; $i++) {
  $port = $portl[$i];
  Invoke-Expression "netsh interface portproxy add v4tov4 listenport=$port connectport=$port connectaddress=$remoteport";
  Invoke-Expression "netsh advfirewall firewall add rule name=$port dir=in action=allow protocol=TCP localport=$port";
}

Invoke-Expression "netsh interface portproxy show v4tov4";