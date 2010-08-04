<?php

function riat_add_relationship($form_state, $name, $chid) {
  $form = array();
  $form['name'] = array(
    '#type' => 'value',
    '#value' => $name,
  );
  $form['pchid'] = array(
    '#type' => 'value',
    '#value' => $chid,
  );
  $relationship = riat_relationship_load($chid, $name);
  $parent = riat_relationship_load($relationship->pchid, $name);
  $plugins = riat_get_relationships(array('parent_entity' => array($parent->object)));
  $form['object'] = array(
    '#type' => 'select',
    '#title' => t('Select an Object'),
    '#options' => $plugins,
  );
  return $form;
}

function riat_delete_relationship($form_state, $name, $chid) {
  
}
