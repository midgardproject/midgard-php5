<?php

class midgard_documentor
{
    var $index_buffer = null;
    var $class_buffer = null;
    var $release = "0.0.1";
    var $release_name = "NoName";

    public function set_release ($release, $release_name)
    {
        $this->release = $release;
        $this->release_name = $release_name;
    }

    public static function get_classes()
    {
        $classes = array();

        $re = new ReflectionExtension("midgard2");
        $classes = $re->getClasses();

        foreach ($classes as $refclass)
        {    
            $mrcs[] = new midgard_reflection_class ($refclass->getName());
        }

        return $mrcs;
    }

    public static function get_midgard_object_classes()
    {
        $classes = array();

        $all_classes = self::get_classes();

        if (empty($all_classes))
            return $classes;

        foreach ($all_classes as $refclass)
        {
            $parent_class = $refclass->getParentClass();

            if (!$parent_class)
            {
                continue;
            }
    
            if ($parent_class->getName() == "midgard_object")
            {
                $classes[] = $refclass;
            }
        } 

        asort($classes);

        return $classes;
       
    }

    public static function get_midgard_dbobject_classes()
    {
        $classes = array();

        $all_classes = self::get_classes();

        if (empty($all_classes))
            return $classes;

        foreach ($all_classes as $refclass)
        {
            $parent_class = $refclass->getParentClass();

            if (!$parent_class)
            {
                continue;
            }
    
            if ($parent_class->getName() == "midgard_dbobject")
            {
                $classes[] = $refclass;
            }
        } 

        return $classes;
    }

    public static function get_midgard_base_classes()
    {
        $classes = array();

        $all_classes = self::get_classes();

        if (empty($all_classes))
            return $classes;

        foreach ($all_classes as $refclass)
        {
            $parent_class = $refclass->getParentClass();

            if ($parent_class) 
            {
                if ($parent_class->getName() == "midgard_object"
                    || $parent_class->getName() == "midgard_dbobject")
                {
                    continue;
                }
            }
             
            $classes[] = $refclass;
        } 

        return $classes;
    }

    public function create_footer()
    {

    }

    private function add_to_index(ReflectionClass $refclass)
    {
        $classname = $refclass->getName();
        $this->index_buffer .= "<a href=\"" . $classname . ".html\">" . $classname . "</a> </br>";
    }

    public function get_index_buffer()
    {
        $this->index_buffer .= "<h2> Base classes </h2>";       
        $midgard_base_classes = self::get_midgard_base_classes();

        foreach ($midgard_base_classes as $refclass)
        {
            $this->add_to_index($refclass);
        }

        $this->index_buffer .= "<h2> Database access classes </h2>";
        $midgard_dbobject_classes = self::get_midgard_dbobject_classes();

        foreach ($midgard_dbobject_classes as $refclass)
        {
            $this->add_to_index($refclass);
        }

        $this->index_buffer .= "<h2> User defined (MgdSchema) classes </h2>";
        $midgard_object_classes = self::get_midgard_object_classes();

        foreach ($midgard_object_classes as $refclass)
        {
            $this->add_to_index($refclass);
        }

        return $this->index_buffer;
    }

    private function get_page_header()
    {
        $header = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n";
        $header .= "<html>\n";
        $header .= "<header> \n";
        $header .= "<link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\">\n";
        $header .= "</head>\n";
        $header .= "<body bgcolor=\"white\" text=\"black\" link=\"#0000FF\" vlink=\"#840084\" alink=\"#0000FF\">\n";

        return $header;
    }

    private function get_page_footer()
    {
        $footer = "</body>\n";
        $footer .= "</html>\n";
    }

    public function create_page ($filepath = null, $buffer = null)
    {
        $page_buffer = self::get_page_header();
        $page_buffer .= $buffer;
        $page_buffer .= self::get_page_footer();

        file_put_contents ($filepath, $page_buffer); 
    }
}

?>
