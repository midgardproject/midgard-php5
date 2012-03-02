<?php

class StorageTest extends MidgardTest
{
    /* Test database initialization */
    public function testCreateBaseStorage()
    {
        $this->assertTrue(MidgardStorage::create_base_storage());
    }

    public function testCreateTypesStorage()
    {
        $re = new ReflectionExtension('midgard2');
        $classes = $re->getClasses();
        foreach ($classes as $refclass) {
            if ($refclass->isAbstract() || $refclass->isInterface()) {
                continue;
            }

            $type = $refclass->getName();
            if (!is_subclass_of($type, 'MidgardDBObject')) {
                continue;
            }

            $this->assertTrue(MidgardStorage::create_class_storage($type));
        }
    }
}

?>
