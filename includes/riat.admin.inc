<?php

function riat_add_relationship($form_state, $name, $chid) {
  ctools_include('dependent');
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
  //$options = riat_get_relationships(array('parent_entity' => array($parent->object)));
  $options = riat_get_relationships();
  $plugins = array();
  foreach ($options as $key => $value) {
    $plugins[$key] = riat_get_relationships(NULL, $key);
  }
  $form['object'] = array(
    '#type' => 'select',
    '#title' => t('Select an Object'),
    '#options' => $options,
  );
  foreach ($plugins as $key => $plugin) {
    if (isset($plugin['add form']) && function_exists($plugin['add form'])) {
      $process = array(
        '#process' => array('ctools_dependent_process'),
        '#dependency' => array('edit-object' => array($key)),
      );
      $form[$key] = array(
        '#type' => 'fieldset',
        '#title' => $plugin['title'],
        '#description' => $plugin['description'],
        '#tree' => TRUE,
        '#prefix' => '<div id="edit-'. implode('-', explode('_', $key)) .'-wrapper"><div id="edit-'. implode('-', explode('_', $key)) .'">',
        '#suffix' => '</div></div>',
        '#input' => TRUE,
      ) + $process;
      $form[$key] += $plugin['add form']($form_state, $relationship, $process);
    }
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Relationship'),
  );
  return $form;
}

function riat_delete_relationship($form_state, $name, $chid) {
  
}
