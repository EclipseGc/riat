<?php

class riat_export_ui extends ctools_export_ui {
  /**
   * Main entry point to manage a relationship.
   */
  function manage_page($js, $input, $item, $step = NULL) {
    drupal_set_title($this->get_page_title('manage', $item));

    // Check to see if there is a cached item to get if we're using the wizard.
    if (!empty($this->plugin['use wizard'])) {
      $cached = $this->edit_cache_get($item, 'edit');
      if (!empty($cached)) {
        $item = $cached;
      }
    }

    $form_state = array(
      'plugin' => $this->plugin,
      'object' => &$this,
      'ajax' => $js,
      'item' => $item,
      'op' => 'manage',
      'form type' => 'manage',
      'rerender' => TRUE,
      'no_redirect' => TRUE,
      'step' => $step,
      // Store these in case additional args are needed.
      'function args' => func_get_args(),
    );

    $output = $this->manage_execute_form($form_state);
    if (!empty($form_state['executed'])) {
      $export_key = $this->plugin['export']['key'];
      drupal_goto(str_replace('%ctools_export_ui', $form_state['item']->{$export_key}, $this->plugin['redirect']['manage']));
    }

    return $output;
  }
  
  /**
   * Execute the form.
   *
   * Add and Edit both funnel into this, but they have a few different
   */
  function manage_execute_form(&$form_state) {
    if (!empty($this->plugin['use wizard'])) {
      return $this->manage_execute_form_wizard($form_state);
    }
    else {
      return $this->manage_execute_form_standard($form_state);
    }
  }

  /**
   * Execute the standard form for managing relationships.
   */
  function manage_execute_form_standard(&$form_state) {
    ctools_include('form');
    $output = ctools_build_form('riat_ui_relationship_item_form', $form_state);
    if (!empty($form_state['executed'])) {
      $this->manage_save_form($form_state);
    }

    return $output;
  }
  
  function manage_form(&$form, &$form_state) {
    // Find the menu path.
    $menu = $form_state['plugin']['menu']['menu prefix'] .'/'. $form_state['plugin']['menu']['menu item'] .'/'. $form_state['plugin']['menu']['items']['manage']['path'];
    // Find the relationship name.
    $ctools_export_ui = array_search('%ctools_export_ui', explode('/', $menu));

    $tree = riat_load_relationship_tree(arg($ctools_export_ui), 'recursive');
    foreach ($tree->raw as $item) {
      $form[$item->chid]['#item'] = $item;
      $form[$item->chid]['title'] = array(
        '#value' => $item->object_type .' '. $item->object .' '. $item->relationship,
      );
      $form[$item->chid]['chid'] = array(
        '#type' => 'hidden',
        '#value' => $item->chid,
      );
      $form[$item->chid]['pchid'] = array(
        '#type' => 'textfield',
        '#default_value' => $item->pchid,
      );
      $form[$item->chid]['weight'] = array(
        '#type' => 'weight',
        '#default_value' => $item->weight,
      );
      //$operations = 
      $form[$item->chid]['operations'] = array(
        '#type' => 'markup',
        '#value' => riat_ui_get_operations($item),
      );
      $form['#theme'] = 'riat_manage_relationship_form';
    }
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
    $form['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Reset to Defaults'),
    );
    return $form;
  }
}

function riat_ui_relationship_item_form(&$form_state) {
  $form = array();
  $form_state['object']->manage_form($form, $form_state);
  return $form;
}