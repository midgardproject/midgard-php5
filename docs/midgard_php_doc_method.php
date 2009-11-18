<?php

require_once 'midgard_php_doc_interface.php';

class midgard_php_doc_method extends midgard_php_doc_class implements midgard_php_doc
{
    private $refmethod = null; 
    private $refclass = null;

    public function __construct ($classname = null, $method = null)
    {
        if ($classname == null || $method == null)
            throw new Exception ("Expected class and method name");
 
        $this->refclass = new midgard_reflection_class ($classname);
        $this->refmethod = new midgard_reflection_method ($classname, $method);
    }

    public function get_doc_buffer()
    {
        $buffer = "<h1>" . $this->refclass->getName() . "</h1>";
        $buffer .= "<h2> Method: ". $this->refmethod->getName() . "</h2>";

        $buffer .= "<p>" . $this->refmethod->getDocComment() . "</p>";

        $isstatic = $this->refmethod->isStatic() ? "static" : "";
        $ispublic = $this->refmethod->isPublic() ? "public" : "";

        $parameters = "";
        $i = 0;

        $params = $this->refmethod->getParameters();
        $required = $this->refmethod->getNumberOfRequiredParameters();
        foreach($params as $param)
        {
            if ($i > 0)
            {
                $parameters .= ", ";
            }
            if ($i >= $required) $parameters .= "[";
            $parameters .= $param->getName();
            if ($i >= $required) $parameters .= "]";
            $i++;
        }
        $buffer .= "<span class=\"reserved_word\">" . $ispublic . " " . $isstatic . "</span> " . $this->refmethod->getName() . " (" . $parameters . ")</br>\n";

	$rr = $this->refmethod->returnsReference();	
	$rrstr = $rr ? "yes" : "no";
	$buffer .= "Returns reference: " . $rrstr; 

        return $buffer;
    }

    protected function get_navi_buffer()
    {
        $buffer = parent::get_navi_buffer();
        $buffer .= "<br />";
        $mpdc = new midgard_php_doc_class ($this->refclass->getName());
        $buffer .= "<strong>Class: </strong> <a href=\"" . $mpdc->get_filename() . "\">" . $this->refclass->getName() . "</a>";
        
        return $buffer;
    }

    public function get_navigation_buffer()
    {
        return parent::get_navigation_buffer();
    }

    public function get_buffer()
    {
        return parent::get_buffer();
    }

    public function get_filename()
    {
        $filename = $this->refclass->getName() . "_" . $this->refmethod->getName() . ".html";
        return $filename;   
    }
}

?>
