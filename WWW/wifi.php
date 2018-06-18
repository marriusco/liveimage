<?php
    $USER="pi";
    $PASW="elamar"; //localhost

    class Ssh
    {
        private $connection;

        public function __construct($h,$u,$p)
        {
            $this->connection = ssh2_connect($h, 22);        
            ssh2_auth_password($this->connection, $u, $p);
        }

        function exec($cmd)
        {
            $stream = ssh2_exec($this->connection, $cmd);
	    stream_set_blocking($stream,true);
            $stream_out = ssh2_fetch_stream($stream, SSH2_STREAM_STDIO);
            $stream_err =  ssh2_fetch_stream($stream, SSH2_STREAM_STDERR);
            while($line = fgets($stream_err)) { flush(); echo "<font color='red'>".$line."</font>\n"; }
            while($line = fgets($stream_out)) { flush(); echo $line."\n";}
            fclose($stream_out);
            fclose($stream_err);

        }

        function flush()
        {
            $stream = ssh2_exec($this->connection, "\r\n");
	    stream_set_blocking($stream,true);
            $stream_out = ssh2_fetch_stream($stream, SSH2_STREAM_STDIO);
            $stream_err =  ssh2_fetch_stream($stream, SSH2_STREAM_STDERR);
            while($line = fgets($stream_err)) { flush();}
            while($line = fgets($stream_out)) { flush();}
            fclose($stream_out);
            fclose($stream_err);
        }

        function shell($cmd, $errors=false)
        {
            $stream = ssh2_exec($this->connection, $cmd);
	    stream_set_blocking($stream,true);
            $stream_out = ssh2_fetch_stream($stream, SSH2_STREAM_STDIO);
            $stream_err = ssh2_fetch_stream($stream, SSH2_STREAM_STDERR);
            stream_set_blocking($stream_out, true);
            stream_set_blocking($stream_err, true);
            $err = stream_get_contents($stream_err);
            $ret = "";
            if($errors){
                if($err != null)
                    $ret .= "<font color='red'>".$err."</font>\n";
            }
            $ret .= stream_get_contents($stream_out);
            fclose($stream_out);
            fclose($stream_err);
            return $ret;
        }
    };

    if(isset($_GET['hapd']))
    {
        $ssh = new Ssh("localhost",$USER,$PASW);
        $hapd = $ssh->shell("ps ax | grep hostapd | grep -v grep");
        if($hapd!=null)
        {
            $ssh->exec("sudo systemctl stop hostapd");
        }
        else
        {
            $ssh->exec("sudo systemctl restart wpa_supplicant");
        }

    }
    else if(isset($_GET['emac']))
    {
        //print_r($_GET);
        //
        $ssh = new Ssh("localhost",$USER,$PASW);
        $qry = "/usr/bin/wpa_passphrase {$_GET['name']} {$_GET['p']}";
        $iface = $_GET['w'];
        $wpc = ($ssh->shell($qry));
        $ssh->exec("sudo systemctl stop wpa_supplicant");
        if($wpc != null){
	    
            $old = file_get_contents("/etc/wpa_supplicant/wpa_supplicant.conf");
            file_put_contents("/etc/wpa_supplicant/wpa_supplicant.conf.$(date)",$old);

            file_put_contents("/etc/wpa_supplicant/wpa_supplicant.conf",$wpc);
            $ssh->exec("sudo systemctl start wpa_supplicant");
            file_put_contents("/etc/wpa_supplicant/env", "WLAN={$iface}");

            $ssh->exec("sudo /sbin/ifconfig {$_GET['w']} down");
            $ssh->exec("sudo /sbin/ifconfig {$_GET['w']} up");
            $ssh->exec("sudo kill -9 $(pidof dhclient)");
            $ssh->exec("sudo /sbin/dhclient {$_GET['w']}");
            $wlan =  $ssh->shell("/sbin/iwconfig | grep ESSID | awk {'print $1}'");
            if($wlan == null)
            {
                echo "There is no wifi interface detected!";
                die();
            }
            // find if there is an ip there
            $ip =  $ssh->shell("/sbin/ifconfig {$wlan} 2>&1 | grep 'inet' | grep 'netmask' | awk '{print $2}'");
            echo "Connected to {$ip}";
	   
        }
        else{
            echo "Error {$qry}";
        }
        die();
    }

?>


<style type = "text/css">
.wifipb{}
.wifikon
{
    border: 1px solid #000;
    cursor: pointer;
    width:100px;    
    display:inline-block;
}

.wifion
{
    border: 1px solid #000;
    width:200px;
    display:inline-block;
    background:#AFA;
}
.hdefault{}

</style>
<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
<script>
$(document).ready(function() {

   $('.hdefault').toggle();

    $(".wifikon").click(function() {
        var id = "h_" + $(this).attr("id");
        $('#' + id).toggle();
    });

    $('#killap').click(function() {

        $(this).html("Processing...");
        $.ajax({async: true,
                type: 'GET',
                url: '/index.php?hapd=1',
                success: function(result)
        {
            location.reload();
        }});
    });


    $(".wifipb").click(function() {

        $(this).html("Connecting...");
        var pbid = $(this).attr('id');
        var emac = $(this).attr('id').substring(2); 
        var pid = "p_" + emac;
        var wid = "w_" + emac;
        var name = $(this).attr('name');
        var pass = $('#'+pid).val();   
    
        $.ajax({async: true,
                type: 'GET',
                url: '/index.php?emac='+emac+'&name='+name+'&p='+pass+'&w='+wid,
                success: function(result)
        {
            $('#'+pbid).html("Refreshing...");
            $('#' +'h_'+ emac).toggle(200);
            location.reload();
        }});
    });
});
</script>
<?php

    $ssh = new Ssh("localhost",$USER,$PASW);
    sleep(1);
    $iscon=null;
    if (is_file("/etc/wpa_supplicant/wpa_supplicant.conf"))
    {
        $ssid = str_replace("\t","",$ssh->shell("grep ssid /etc/wpa_supplicant/wpa_supplicant.conf")); 
        $ssid = str_replace("\"","",$ssid); 
        $ssid = str_replace("=","",$ssid); 
        $ssid = str_replace("\n","",$ssid); 
        $iscon = str_replace("ssid","",$ssid); 
    }
    $ssh->exec("sudo systemctl restart wpa_supplicant");
    $running = $ssh->shell("ps ax | grep supplic | grep -v grep");
    if($running == null)
    {
        echo "<font color='red'>Cannot start wpa_suplicant. F5 to refresh !</font>";
        die();
    }    
    // find out wlan
    $wlan =  $ssh->shell("/sbin/iwconfig 2>&1 | grep ESSID | awk {'print $1}'");
    if($wlan == null)
    {
        echo "There is no wifi interface detected. F5 to refresh !";
        scan($iscon, $ssh, $wlan ,"*");
    }
    $wlan=str_replace("\n","",$wlan);
    // find if there is an ip there
    $cmd =     "/sbin/ifconfig {$wlan} 2>&1 | grep 'inet' | grep 'netmask' | awk '{print $2}'";
    $ipaddr =  str_replace(" ","",$ssh->shell($cmd));
    if($ipaddr==null)
    {
        echo "There is no IP !";
        
    }
    scan($iscon, $ssh, $wlan, $ipaddr);

function scan($iscon, &$ssh, $wlan,$ipaddr)
{    

    $running = $ssh->shell("ps ax | grep supplic | grep -v grep");
    if($running == null)
    {
        echo "<font color='red'>Cannot start wpa_suplicant. F5 to refresh !</font>";
        die();
    }    

    $keepgoing=false;
    $enabled=false;
    $qual=0;
    $name=null;
    $enc="off";
    $enc="<font color='red'>Open</font>";
    $keepgoing=false;
    $f="";

    // find on which we are connected
    $alines = explode("\n",$ssh->shell("/sbin/iwlist scanning"));
    $map = array();
    echo "<table width=800px' border='1' cellspacing='0' cellpading='0'><tr>".
         "<th width='30%'>Network</th><th width='20%'>Signal</th><th width='20%'>Security</th><th width='30%'>Status</th></tr>";
    foreach ($alines as $l)
    {
        if(strstr($l,"Cell"))
        {
            $enabled=false;
            $qual=-1;
            $name=null;
            $enc="off";
            $enc="<font color='red'>Open Network</font>";
            $keepgoing=false;
            $pl=explode(" ", $l);
            $f=$pl[14];
        }
        if($keepgoing){
            continue;
        }
        if(strstr($l,"ality"))
        {
            $qual = $l;
        }
        else if(strstr($l,"ESSID"))
        {
            $ghi=strpos($l,"\"");
            $name=str_replace("\"","",substr($l,$ghi+1));
            //$name.=" ".$mac . " - ".$iscon ;
        }
        else if(strstr($l,"WPA") || strstr($l,"IEEE"))
        {
            $enabled=true;
        }
        else if(strstr($l,"Encryption key"))
        {
            $enc="WPA Secured";
        }

/*
   Cell 52 - Address: 2C:30:33:E1:45:D6
                    ESSID:"ethanPlace"
                    Protocol:IEEE 802.11bgn
                    Mode:Master
                    Frequency:2.422 GHz (Channel 3)
                    Encryption key:on
                    Bit Rates:144 Mb/s
                    Extra:rsn_ie=30140100000fac040100000fac040100000fac020c00
                    IE: IEEE 802.11i/WPA2 Version 1
                        Group Cipher : CCMP
                        Pairwise Ciphers (1) : CCMP
                        Authentication Suites (1) : PSK
                    IE: Unknown: DD310050F204104A000110104400010210470010F698DD6DB3B4EB52AEB8DC54CC242CAA103C0001031049000600372A000120
                    Quality=25/100  Signal level=26/100  
                    Extra:fm=0001

*/

        if($enabled && $name && $qual!=-1)
        {
            if(isset($map[$name]))
                continue;
            $map[$name]=1;

            echo "<tr><td>{$name}</td><td>";

            $perc = substr($qual, strpos($qual,"Quality=")+8);
            $perc = substr($perc, 0, strpos($perc,"/"));

            echo "<div class='meter-value' style='background-color:#393;width:{$perc}%;'>".
                  "{$perc}%</div>";
            echo "</td><td>{$enc}</td>";
            if($name==$iscon)
            {
                echo "<td><div class='wifion'>Connected: {$ipaddr}</div></td></tr>";
            }
            else if($perc==0)
            {
                echo "<td>0 Signal</td></tr>";
            }
            else
            {
                $fid = str_replace(":","_",$f);
                echo "<td width='40%'><div class='wifikon' id='{$fid}' >Connect</div>";
                    echo "<div class='hdefault' id='h_{$fid}'>";
                    echo "<li>Password:<input name='password'  id='p_{$fid}' value=''  size='12'>".
                         "<input hidden id='w_{$fid}' value='{$wlan}'>";
                    echo "<li><button class='wifipb' id='m_{$fid}' name='{$name}'>Connect...</button></div>";

                echo "</td></tr>";
            }
            $keepgoing=true;
       }
    }
    echo "<tr><td colspan='4'>";
    $hapd = $ssh->shell("ps ax | grep hostapd | grep -v grep");
    if($hapd!=null)
    {
        echo "HostAP can disrupt wifi if the wifi card does not run in dual mode: <button id='killap'>Kill Wifi Hot Spot</button>";
        $hapd=explode(" ",$hapd);
        foreach($hapd as $h)
        {
            if(strstr($h,".conf"))
            {
                $cfgfile = $h;
                break;
            }
        }
        if($cfgfile!=null)
        {
            $iface = explode("=",str_replace("\n","",$ssh->shell("grep interface {$cfgfile}")));
            $iph = $ssh->shell("/sbin/ifconfig {$iface[1]} | grep netmask | awk '{print $2}'");

            echo "{$iface[1]} {$iph}";
        }
        
    }
    else
    {
       
        echo "HostAP is not running: <button id='killap'>Start Wifi Hot Spot</button>";
    }    
    echo "</td></tr></table>";
    die();
}
?>



<!-- 

[Unit]
Description=WPA supplicant
Before=network.target
After=dbus.service
Wants=network.target

[Service]
Type=dbus
EnvironmentFile=/etc/wpa_supplicant/env
BusName=fi.epitest.hostap.WPASupplicant
#ExecStart=/sbin/wpa_supplicant -u -s -O /run/wpa_supplicant -c/etc/wpa_supplicant/wpa_supplicant.conf
ExecStart=/sbin/wpa_supplicant -B -i${WLAN} -c/etc/wpa_supplicant/wpa_supplicant.conf -Dwext

[Install]
WantedBy=multi-user.target
Alias=dbus-fi.epitest.hostap.WPASupplicant.service


-->




