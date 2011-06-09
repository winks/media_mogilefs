<?php
/**
 * A PHP Stream Wrapper for MogileFS
 *  by Florian Anderiasch <fa at art-core dot org>
 *  inspired by media_amazon and hash_wrapper
 *
 */
class MediaMogilefsStreamWrapper implements DrupalStreamWrapperInterface
{
    protected $_uri;

    protected $_mog;

    protected $_pos;

    protected $_key;

    protected $_object;

    protected $_m = array(
        'options' => array(
            'socketTimeout' => 5,
            'streamTimeoutSeconds' => 5,
        ),
    );

    public function __construct()
    {
        if(!isset($this->_m['hosts'])) {
            $this->_m['hosts'] = variable_get('media_mogilefs_hosts', '');
            if (!is_array($this->_m['hosts'])) {
                $this->_m['hosts'] = explode(';', $this->_m['hosts']);
            }
        }
        if(!isset($this->_m['domain'])) {
            $this->_m['domain'] = variable_get('media_mogilefs_domain', '');
        }
        if(!isset($this->_m['class'])) {
            $this->_m['class'] = variable_get('media_mogilefs_class', '');
        }
    }

    public function setUri($uri)
    {
        $this->_uri = $uri;
        error_log("uri is $uri".PHP_EOL);
        $parts = explode('/', $uri);
        $base = explode('\.', array_pop($parts));
        $this->_key = $base[0]."";
        error_log("key is $this->_key ".PHP_EOL);
    }

    public function getUri()
    {
        return $this->_uri;
    }

    //@TODO
    public function getExternalUrl()
    {
        // $proto $host $key $size $ext
        $path = '%s/%s/%s';
        return sprintf(
            $path,
            $GLOBALS['base_url'],
            'mog',
            $this->_key
        );
    }

    //@TODO
    public static function getMimeType($uri, $mapping = NULL)
    {
        if (!isset($mapping)) {
            $mapping = 0;
        }
        return false;
    }

    public function chmod($mode)
    {
        return true;
    }

    //@TODO
    public function realpath()
    {
        return sprintf('mogilefs://%s', "");
    }

    public function dirname($uri = null)
    {
        list($stream, $target) = explode('://', $uri, 2);
        $target = 1; //@TODO
        $dirname = dirname($target);

        if ($dirname == '.') {
            $dirname = '';
        }

        return sprintf('%s://%s', $stream, $dirname);
    }

    public function stream_open($uri, $mode, $options, &$opened_url)
    {
        $this->setUri($uri);
        if ($this->_getMogileClient()) {
            $this->_pos = 0;
            error_log("opened $uri".PHP_EOL);
            return true;
        }
    }

    public function stream_close()
    {
        $this->_pos = 0;
        $this->_object = 0;
        return true;
    }

    public function stream_lock($operation)
    {
        return lock_aquire('media_mogilefs:' . $this->_uri);
    }

    public function stream_read($count)
    {
        if ($this->_getMogileClient()) {
            error_log("reading $this->_pos $count".PHP_EOL);
            $data = substr($this->_getObject(), $this->_pos, $count);
            $this->_pos += $count;
            return $data;
        }
        return false;
    }

    public function stream_write($data)
    {
        //@TODO
        if ($this->_getMogileClient() && strlen($data) > 0) {
            error_log("putting to key $this->_key".PHP_EOL);
            try {
                $this->_getMogileClient()->storeData($this->_key, $this->_m['class'], $data);
            } catch (Exception $e) {
                return 0;
            }
            return strlen($data);
        }
        return false;
    }

    public function stream_eof()
    {
        if ($this->_getMogileClient() && $tmp = $this->_getObject()) {
            return $this->_pos >= strlen($tmp);
        }
        return true;
    }

    public function stream_seek($offset, $whence)
    {
        switch ($whence) {
            case SEEK_SET:
                if ($this->_getMogileClient() && $tmp = $this->_getObject()) {
                    if ($offset < strlen($tmp) && $offset >= 0) {
                        $this->_pos = $offset;
                        return true;
                    }
                }
            case SEEK_CUR:
                if ($offset >= 0) {
                    $this->_pos += $offset;
                    return true;
                }
            case SEEK_END:
                if ($this->_getMogileClient() && $tmp = $this->_getObject()) {
                    if (strlen($tmp) + $offset >= 0) {
                        $this->_pos = strlen($tmp) + $offset;
                        return true;
                    }
                }
            default:
                return false;
        }
        return false;
    }

    public function stream_flush()
    {
        return true;
    }

    public function stream_tell()
    {
        return $this->_pos;
    }

    public function stream_stat()
    {}

    public function unlink($uri)
    {
        if ($uri == $this->_uri) {
            $this->_mog->delete($this->_key);
            return true;
        }
        return false;
    }

    public function rename($from_uri, $to_uri)
    {
        return false;
        //$this->_mog->rename($this->_key, $newkey);
    }

    public function mkdir($uri, $mode, $options)
    {
        return true;
    }

    public function rmdir($uri, $options)
    {
        return false;
    }

    public function url_stat($uri, $flags)
    {}

    protected function _getMogileClient()
    {
        if (is_null($this->_mog)) {
            if (!class_exists('File_Mogile')) {
                require 'File/Mogile.php';
            }
            try {
                $this->_mog = new File_Mogile(
                    $this->_m['hosts'],
                    $this->_m['domain'],
                    $this->_m['options']
                );
            }
            catch (RequestCore_Exception $e) {
                drupal_set_message('There was a problem connection to MogileFS', 'error');
            }
            catch (Exception $e) {
                drupal_set_message('There was a problem using MogileFS: ' . $e->getMessage(), 'error');
            }
        }
        return $this->_mog;
    }

    protected function _getObject() {
        if (!$this->_object) {
            try {
                $this->_object = $this->_mog->getFileData($this->_key);
            } catch (Exception $e) {
                return null;
            }
        }
        return $this->_object;
    }

    public function dir_opendir($uri, $options)
    {
        return true;
    }

    public function dir_readdir()
    {
        return false;
    }

    public function dir_rewinddir()
    {
        return true;
    }

    public function dir_closedir()
    {
        return true;
    }
}
