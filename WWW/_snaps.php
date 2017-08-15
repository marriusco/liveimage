<?php

$files = array();
$dir   = new DirectoryIterator('./snaps');
foreach ($dir as $fileinfo)
{
    if($fileinfo->getExtension()=="jpg")
        $files[$fileinfo->getMTime()][] = $fileinfo->getFilename();
}
ksort($files);
$nfiles=count($files);
$visible=8;

echo "<table class='tg'><tr>";
$Start=$_GET['s'];
$date = date('Y-m-d H:i:s');
$start_date = new DateTime($date);
$Snaps = 0;
foreach($files as $date=>$file)
{
    $datex = date('Y-m-d H:i:s', $date);
    $whenago = $start_date->diff(new DateTime($datex));
    $nsince = $whenago->days ."D/". $whenago->h . "H Ago";

    if($nsince != $Start)
        continue;
    $datex = date('Y-m-d H:i:s', $date);
    if(is_array($file))
    {
        foreach($file as $fil)
        {
            echo "<td class='tg-031e'><img class='clickm' src='./snaps/{$fil}' width=130;/><br />{$datex}</td>";
            ++$Snaps;
        }
    }
    else
    {
        echo "<td class='tg-031e'><img class='clickm' src='./snaps/{$fil}' width=130;/><br />{$datex}</td>";
        ++$Snaps;
    }
}
echo "</tr></table>";
echo "<div id='snaps' hidden>{$Snaps}</div>\n";
?>
