<?php
require 'File/Mogile.php';

$m = new File_Mogile(
    array('localhost:7001'),
    'datastore',
    array(
        'socketTimeout' => 5,
        'streamTimeoutSeconds' => 5,
    )
);

$key = isset($_GET['key']) ? $_GET['key'] : false;

if ($key) {
    $data = $m->getFileData($key);
    $ext = array_pop(explode('.', $key));
    $s = sprintf('Content-type: image/%s',$ext);
    error_log($s);
    header($s);
    echo $data;
}
