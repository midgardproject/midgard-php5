<?php

require_once 'midgard_php_doc_interface.php';

class midgard_php_doc_class implements midgard_php_doc
{
    private $refclass = null;    

    public function __construct ($classname = null)
    {
        if ($classname == null)
            throw new Exception ("Expected classname");

        $this->refclass = new midgard_reflection_class ($classname);
    }

    public function get_doc_method ($method)
    {
        return new midgard_php_doc_method ($this->refclass->getName(), $method);
    }

    public function get_doc_buffer ()
    {
        $buffer = "<h1> Class: ". $this->refclass->getName () . "</h1>";
        $buffer .= "<p>" . $this->refclass->getDocComment () . "</p>";

        $buffer .= "<h2>Methods</h2>";
        $methods = $this->refclass->getMethods();

        foreach ($methods as $refmethod)
        {
            $isstatic = $refmethod->isStatic() ? "static" : "";
            $ispublic = $refmethod->isPublic() ? "public" : "";

            $parameters = "";
            $i = 0;

            $params = $refmethod->getParameters();
            $required = $refmethod->getNumberOfRequiredParameters();

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
            
            $mpdm = $this->get_doc_method ($refmethod->getName());
            $buffer .= "<span class=\"reserved_word\">" . $ispublic . " " . $isstatic . "</span> <a href=\"" . $mpdm->get_filename ($refmethod) . "\">" . $refmethod->getName() . "</a> (" . $parameters . ")</br>\n";
        }

        $buffer .= "<h2>Signals</h2>";
        $signals = $this->refclass->listSignals();

        foreach($signals as $signal)
        {
            $buffer .= $signal . "</br>";
        }

        return $buffer;

    }

    protected function get_navi_buffer()
    {
        $buffer = "<a href=\"index.html\">Home</a>";
        return $buffer;
    }    

    public function get_navigation_buffer()
    {
        $buffer = "<div class=\"navigation\">";
        $buffer .= $this->get_navi_buffer();
        $buffer .= "</div>\n";
    
        return $buffer;
    }

    public function get_buffer()
    {
        $buffer = $this->get_navigation_buffer() . $this->get_doc_buffer();
        return $buffer;
    }

    public function get_filename()
    {
        $filename = $this->refclass->getName() . ".html";
        return $filename;
    }
}

?>
