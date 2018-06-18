<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <link rel="stylesheet" href="snaps.css">
    <script type="text/javascript" src="jq/jquery-1.11.1.min.js"></script>

</head>
<body>

<div class="top-bar">
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

$count = 0;
$date = date('Y-m-d H:i:s');
$start_date = new DateTime($date);
echo "<div id='out'>Today: {$date}</div>";

//echo "<div class='scrolldiv'><table class='tg'><tr>\n";
echo "<div class='bstimeslider'>\n";
echo "    <div id='rightArrow'>>></div>\n";
echo "    <div id='viewContainer'>\n";
echo "        <div id='tslshow'>\n";
echo "           <table class='tg'><tr>\n";

$since="";
$timgs = 1;
$Date=0;
$RDate=0;
$Whenago=0;
$Groups=0;
$Start = isset($_GET['s']) ? $_GET['s'] : 0;
foreach($files as $date=>$file)
{
    $datex = date('Y-m-d H:i:s', $date);
    $whenago = $start_date->diff(new DateTime($datex));
    $nsince = $whenago->days ."D/". $whenago->h . "H Ago";
    if($Start==0) $Start=$date;

    if($since=="")
        $since=$nsince;
    if($nsince == $since)
    {
        if(is_array($file))
            $timgs += count($file);
        else
            $timgs += 1;
        $Date=$datex;
        $Whenago=$nsince;
        $RDate = $since;
    }
    else
    {
        $since = $nsince;
        echo "<td class='tgdat' id='{$RDate}' title={$RDate}>{$Date}<hr />{$Whenago}<hr />{$timgs} imgs</td>\n";
        $timgs = 0;
        ++$Groups;
    }
}
if($timgs)
{
    echo "<td class='tgdat' id='{$RDate}' title={$RDate}>{$datex}<hr />{$since}<hr />{$timgs} imgs</td>\n";
    ++$Groups;
}
echo "</tr></table>";
echo "</div>";
echo "</div>";
echo "<div id='leftArrow'><<</div>";
echo "</div>\n";

echo "<div id='groups' hidden>{$Groups}</div>\n";

echo "<div class='bstimeslider2'>\n";
echo "    <div id='rightArrow2'>>></div>\n";
echo "    <div id='viewContainer2'>\n";
echo "        <div id='tslshow2'>\n";
echo "           <table class='tg'><tr>\n";

$Snaps = 0;
foreach($files as $date=>$file)
{
    if($date != $Start)

        continue;
    $datex = date('Y-m-d H:i:s', $date);
    if(is_array($file))
    {
        foreach($file as $fil)
        {
            echo "<td class='tg-031e'><img class='clickm' src='_resz.php?f=./snaps/{$fil}' width=140;/><br />{$datex}</td>\n";
//            echo "<td class='tg-031e'><img class='clickm' src='./snaps/{$fil}' width=140;/><br />{$datex}</td>\n";
            ++$Snaps;
        }
    }
    else
    {
//        echo "<td class='tg-031e'><img class='clickm' src='./snaps/{$fil}' width=140;/><br />{$datex}</td>\n";
        echo "<td class='tg-031e'><img class='clickm' src='_resz.php?f=./snaps/{$fil}' width=140;/><br />{$datex}</td>\n";
        ++$Snaps;
    }
}
echo "</tr></table>\n";
echo "<div id='snaps' hidden>{$Snaps}</div>\n";
echo "</div>\n";
echo "</div>\n";
echo "<div id='leftArrow2'><<</div>\n";
echo "</div>\n";
?>
</div>
<img class='preview' id='detail' src="snaps/img_100.jpg" alt="detail" />

</body>
<script>

$(document).ready(function(){


    $('.tgdat').click(function() {
        var id = $(this).attr('id');
        $.ajax({
          url: "_snaps.php?s="+id,
          cache: false,
          success: function(html)
          {
                $("#tslshow2").html(html);
                rescroll();
          }
        });

    });

    var view = $("#tslshow");
    var move = "600px";
    var sliderLimit = (0 - $('#groups').html()) * 140;

    $('#gs').html(sliderLimit);

    $("#rightArrow").click(function(){

        var currentPosition = parseInt(view.css("left"));
        if (currentPosition >= sliderLimit) view.stop(false,true).animate({left:"-="+move},{ duration: 400})

    });

    $("#leftArrow").click(function(){

        var currentPosition = parseInt(view.css("left"));
        if (currentPosition < 0) view.stop(false,true).animate({left:"+="+move},{ duration: 400})

    });

    rescroll();

    function rescroll()
    {

        var view2 = $("#tslshow2");
        var move2 = "600px";
        var sliderLimit2 = (0 - $('#snaps').html())  * 140;

        $('#ss').html(sliderLimit2);

        $("#rightArrow2").click(function(){

            var currentPosition = parseInt(view2.css("left"));
            if (currentPosition >= sliderLimit2) view2.stop(false,true).animate({left:"-="+move2},{ duration: 200})

        });

        $("#leftArrow2").click(function(){

            var currentPosition = parseInt(view2.css("left"));
            if (currentPosition < 0) view2.stop(false,true).animate({left:"+="+move2},{ duration: 200})

        });


        $('.clickm').click(function(){
            //$('#out').html("X" + $(this).attr("src"));
           var src=$(this).attr("src").split("=");
            $("#detail").attr("src", src[1]);
        });

    }
});


</script>
</html>
