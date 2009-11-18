<?php

require_once 'midgard_documentor.php';
require_once 'midgard_php_doc_class.php';
require_once 'midgard_php_doc_method.php';

define ('HTML_DIR', "html");

$md = new midgard_documentor();

$idx_buffer = $md->get_index_buffer();

if (!file_exists(HTML_DIR))
    mkdir(HTML_DIR);

file_put_contents(HTML_DIR . "/index.html", $idx_buffer);

$classes = midgard_documentor::get_classes();

if (empty($classes))
{
    return;
}

foreach ($classes as $refclass)
{
    $mpdc = new midgard_php_doc_class ($refclass->getName());
    $class_filename = HTML_DIR . "/" . $mpdc->get_filename();
    $md->create_page ($class_filename, $mpdc->get_buffer());

    $methods = $refclass->getMethods();
    foreach ($methods as $refmethod)
    {
        $mpdm = new midgard_php_doc_method ($refclass->getName(), $refmethod->getName());
        $method_filename = HTML_DIR . "/" . $mpdm->get_filename();
        $md->create_page ($method_filename, $mpdm->get_buffer());        
    }
}

?>
