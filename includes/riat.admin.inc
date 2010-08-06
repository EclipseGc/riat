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
  $options = riat_get_relationships(array('parent_entity' => array($parent->object)));
  $plugins = array();
  foreach ($options as $key => $value) {
    $plugins[$key] = riat_get_relationships(NULL, $key);
  }
  $form['relationship'] = array(
    '#type' => 'select',
    '#title' => t('Select a relationship'),
    '#options' => $options,
  );
  foreach ($plugins as $key => $plugin) {
    if (isset($plugin['add form']) && function_exists($plugin['add form'])) {
      $process = array(
        '#process' => array('ctools_dependent_process'),
        '#dependency' => array('edit-relationship' => array($key)),
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
      $form[$key]['object'] = array(
        '#type' => 'value',
        '#value' => $plugin['child entity'],
      );
      $form[$key] += $plugin['add form']($form_state, $relationship, $process);
    }
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Relationship'),
  );
  $form['#submit'][] = 'riat_add_relationship_submit';
  return $form;
}

function riat_add_relationship_submit($form, &$form_state) {
  $chid = db_result(db_query("SELECT chid FROM {riat_tree_definition} WHERE name = '%s' ORDER BY chid DESC", $form_state['values']['name']));
  $chid = $chid ? ($chid = $chid + 1) : 1;
  if ($form_state['values']['pchid'] == 0) {
    $depth = 1;
  }
  else {
    $depth = db_result(db_query("SELECT depth FROM {riat_tree_definition} WHERE name = '%s' AND chid = %d ORDER BY chid DESC", $form_state['values']['name'], $form_state['values']['pchid']));
    $depth++;
  }
  $object = new stdClass();
  $object->chid = $chid;
  $object->pchid = $form_state['values']['pchid'];
  $object->name = $form_state['values']['name'];
  $object->object = $form_state['values'][$form_state['values']['relationship']]['object'];
  unset($form_state['values'][$form_state['values']['relationship']]['object']);
  $object->conf = $form_state['values'][$form_state['values']['relationship']];
  $object->relationship = $form_state['values']['relationship'];
  $object->depth = $depth;
  $object->weight = 0;
  drupal_write_record('riat_tree_definition', $object);
}

function riat_delete_relationship($form_state, $name, $chid) {
  
}
