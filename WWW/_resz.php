<?php
if(isset($_GET['f']))
{
//	readfile($_GET['f']);
	$file=$_GET['f']; 
	list($w, $h) = getimagesize($file);	
	$src = imagecreatefromjpeg($file);
	$dst = imagecreatetruecolor(64, 48);
	imagecopyresampled($dst, $src, 0, 0, 0, 0, 128, 96, $w, $h);
	imagejpeg( $dst);
}
?>
