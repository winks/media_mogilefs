<?php
ini_set('display_errors', 'on');
error_reporting(E_ALL);

function variable_get($name, $def) {
    if ($name == 'media_mogilefs_class' || $name == 'media_mogilefs_domain') {
        return 'datastore';
    } else {
        return 'localhost:7001';
    }
}

require 'File/Mogile/Exception.php';
require 'File/Mogile.php';
require '../../../../includes/stream_wrappers.inc';
require './MediaMogilefsStreamWrapper.inc';

stream_wrapper_register('mogilefs', 'MediaMogilefsStreamWrapper');

$url = 'mogilefs://hosts';


$x = file_get_contents($url);

var_dump(strlen($x));

file_put_contents('./x.gif', $x);

file_put_contents('mogilefs://my_awesome_test', $x);
