<?php

function riat_load_relationship_types($type = NULL) {
  $types = module_invoke_all('riat_relationships');
  if (is_null($type)) {
    return $types;
  }
  else {
    return $types[$type];
  }
}

/**
 * Loads various relationship elements.
 *
 * @type
 *  'full'
 *    Load an entire relationship tree, this is the default functionality.
 *  'branch'
 *    Loads only the relationship components within this tree that directly lead to this branch.  Does not load subsequent relationship components.
 *    @a3 = child id of desired component.
 *  'depth'
 *    Loads all relationship components to a certain depth.
 *    @a3 = desired depth.
 *  'single'
 *    Load JUST an individual chid's definition, plus the main relationship definition itself.
 *    @a3 = child id of desired component.
 */
function riat_load_relationship_tree($relationship, $type = 'full', $a3 = NULL) {
  $definition = db_fetch_object(db_query("SELECT * FROM {riat_definition} WHERE name = '%s'", $relationship));
  switch ($type) {
    case 'full':
      $query = db_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' ORDER BY depth", $relationship);
      while ($result = db_fetch_object($query)) {
        $definition->tree[$result->pchid][$result->chid] = $result;
        $definition->raw[$result->chid] = $result;
      }
      break;
    case 'recursive':
      $args = array(
        'name' => $relationship,
        'chid' => $result->chid,
      );
      $results = array();
      riat_recursive_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' AND pchid = %d", $args, 'chid', $results);
      foreach ($results as $result) {
        $definition->tree[$result->pchid][$result->chid] = $result;
        $definition->raw[$result->chid] = $result;
      }
      break;
    case 'branch':
      $args = array(
        'name' => $relationship,
        'pchid' => $a3,
      );
      $results = array();
      riat_recursive_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' AND chid = %d", $args, 'pchid', $results);
      foreach ($results as $result) {
        $definition->tree[$result->pchid][$result->chid] = $result;
        $definition->raw[$result->chid] = $result;
      }
      break;
    case 'depth':
      $query = db_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' AND depth <= %d", $relationship, $a3);
      while ($result = db_fetch_object($query)) {
        $definition->tree[$result->pchid][$result->chid] = $result;
        $definition->raw[$result->chid] = $result;
      }
      break;
    case 'single':
      $result = db_fetch_object(db_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' AND chid = %d", $relationship, $a3));
      $definition->raw[$result->chid] = $result;
  }
  drupal_alter('riat_relationship_definition', $definition, $type, $relationship, $a3);
  return $definition;
}

/**
 * Generically loads up all direct relationships of an object by id.
 *
 * @object
 *  A string like 'node' or 'user'.
 * @id
 *  The unique id of the above object type.
 */
function riat_load_object_relationships($object, $id) {
  $results = array();
  $query = db_query("SELECT * FROM {riat_direct_relationship} WHERE object = '%s', id = %d", $object, $id);
  while ($result = db_fetch_object($query)) {
    $results[$result->name] = $result;
  }
  return $results;
}

/**
 * A handy generic function for doing simple recursive queries.
 * This function is designed to start some distance down a
 * hierarchical tree and work its way back on a single branch.
 * Top level arguments are expected to be 0, commonly following
 * the pchid element for most riat queries.
 *
 * @query_str
 *  The string intended to be run through db_query.  This query should only return a single result.
 * @args
 *  A keyed array of arguments to be passed to our query.  The same $key passed to us will be reset to new $result->$key of the query in $args[$key] = $result->$key format.
 * @key
 *  The key of the element to pass into subsequent queries as $args[$key].
 * @results
 *  An array of previous results passed by reference.
 */

function riat_recursive_query($query_str, $args, $key, &$results) {
  $query = db_query($query_str, $args);
  while ($result = db_fetch_object($query)) {
    $results[] = $result;
    if ($result->$key) {
      $args[$key] = $result->$key;
      riat_recursive_query($query_str, $args, $key, $results);
    }
  }
}

function riat_relationship_load($chid, $name) {
  if ($chid) {
    return db_fetch_object(db_query("SELECT * FROM {riat_tree_definition} WHERE name = '%s' AND chid = %d", $name, $chid));
  }
  else {
    return db_fetch_object(db_query("SELECT * FROM {riat_definition} WHERE name = '%s'", $name));
  }
}

function riat_get_relationships($from = NULL) {
  $relationships = module_invoke_all('riat_relationships');
  foreach ($relationships as $key => $relationship) {
    if ($relationship['entity']['from'][0] == $from) {
      $results[$key] = $relationship['title'];
    }
  }
  return $results;
}

function riat_relationship_export($object, $indent) {
  $relationship = riat_load_relationship_tree($object->name, 'recursive');

  $schema = ctools_export_get_schema($object->table);
  if (!isset($identifier)) {
    $identifier = $schema['export']['identifier'];
  }

  $output = $indent . '$' . $identifier . ' = new ' . get_class($object) . ";\n";

  if ($schema['export']['can disable']) {
    $output .= $indent . '$' . $identifier . '->disabled = FALSE; /* Edit this to true to make a default ' . $identifier . ' disabled initially */' . "\n";
  }
  if (!empty($schema['export']['api']['current_version'])) {
    $output .= $indent . '$' . $identifier . '->api_version = ' . $schema['export']['api']['current_version'] . ";\n";
  }

  $fields = $schema['fields'];
  if (!empty($schema['join'])) {
    foreach ($schema['join'] as $join) {
      if (!empty($join['load'])) {
        foreach ($join['load'] as $join_field) {
          $fields[$join_field] = $join['fields'][$join_field];
        }
      }
    }
  }

  // Go through our schema and joined tables and build correlations.
  foreach ($fields as $field => $info) {
    if (!empty($info['no export'])) {
      continue;
    }
    if (!isset($object->$field)) {
      if (isset($info['default'])) {
        $object->$field = $info['default'];
      }
      else {
        $object->$field = '';
      }
    }

    // Note: This is the *field* export callback, not the table one!
    if (!empty($info['export callback']) && function_exists($info['export callback'])) {
      $output .= $indent . '$' . $identifier . '->' . $field . ' = ' . $info['export callback']($object, $field, $value, $indent) . ";\n";
    }
    else {
      $value = $object->$field;
      if ($info['type'] == 'int') {
        $value = (isset($info['size']) && $info['size'] == 'tiny') ? (bool) $value : (int) $value;
      }

      $output .= $indent . '$' . $identifier . '->' . $field . ' = ' . ctools_var_export($value, $indent) . ";\n";
    }
  }
  // Put relationships here:
  $relationships = $relationship->raw ? $relationship->raw : array();
  foreach ($relationships as $field => $value) {
    $output .= $indent . '$' . $identifier . '->relationships[' . $field . '] = ' . ctools_var_export((array) $value, $indent) . ";\n";
  }

  return $output;
}

function riat_relationship_import($object, $indent) {
  //drupal_set_message('<pre>'. print_r($object, TRUE) .'</pre>');
}

function riat_relationship_delete($object) {
  db_query("DELETE FROM {riat_definition} WHERE name = '%s'", $object->name);
  db_query("DELETE FROM {riat_tree_definition} WHERE name = '%s'", $object->name);
}
