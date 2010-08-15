<?php

function riat_ui_add_relationship_submit($form, &$form_state) {
  $name = $form_state['values']['name'];
  $form_state['redirect'] = "admin/build/relationships/list/$name/manage";
}