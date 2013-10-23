<?php

function adef_camps_register($node) {
  drupal_add_js(drupal_get_path('module', 'adef_camps') . '/js/adef_camps_registration_interface.js');

  $period_date = adef_camps_get_period_date();
  $views_date_arg = $period_date['year'] . '-' . $period_date['month'] . '-' . $period_date['day'] . '--P2D';
  $sessions = views_get_view_result('content_blocks', 'block_sessions', $views_date_arg);
  $user_sessions = views_get_view_result('content_blocks', 'block_sessions', $views_date_arg, $node->nid);
  //user_sessrions[i]->nid
  $selected = array();
  foreach ($user_sessions as $timespace) {
    $selected[] = $timespace->nid;
  }
  $selected_string = implode(',', $selected);
  $form = drupal_get_form('adef_camps_registration_form', $node, $selected_string);
//  $page['registered'] = array(
//      '#type' => 'markup',
//      '#markup' => '<div id="registered-sessions">' . views_embed_view('content_blocks', 'block_sessions', $views_date_arg, $node->nid) . '</div>',
//  );
  $page['available'] = array(
      '#type' => 'markup',
      '#markup' => '<div id="available-sessions">' . views_embed_view('content_blocks', 'block_sessions', $views_date_arg) . '</div>',
  );
  $page['form'] = $form;
  $page['attended'] = array(
      '#type' => 'markup',
      '#markup' => '<div id="attended">' . views_embed_view('content_blocks', 'block_participant_attendance', $node->nid) . '</div>',
  );
  return $page;
}

function adef_camps_registry($node, $type) {
  drupal_add_js(drupal_get_path('module', 'adef_camps') . '/js/adef_camps_registry_entry_interface.js');

  switch ($type) {
    case 'achievement':
      $title_suffix = t('Achievments');
      break;
    case 'incident':
      $title_suffix = t('Incidents');
      break;
  }
  drupal_set_title(drupal_get_title() . ": " . $title_suffix, CHECK_PLAIN);

  $page['form'] = drupal_get_form('adef_camps_registery_entry_form', $node, $type);
  $page['log'] = array(
      '#type' => 'markup',
      '#markup' => '<div id="l">' . views_embed_view('content_blocks', 'block_participant_registry', $node->nid, $type) . '</div>',
  );
  return $page;
}

function adef_camps_schedule($node) {
  $period_date = adef_camps_get_period_date();
  $views_date_arg = $period_date['year'] . '-' . $period_date['month'] . '-' . $period_date['day'] . '--P2D';
  $page['schedule'] = array(
      '#type' => 'markup',
      '#markup' => '<div id="participant-schedule">' . views_embed_view('content_blocks', 'block_participant_schedule', $node->nid) . '</div>',
  );
  return $page;
}

function adef_camps_registration_form($form, &$form_state, $node, $selected) {
  $form = array(
      'timespace' => array(
          '#type' => 'hidden',
          '#default_value' => $selected,
      ),
      'old_timespace' => array(
          '#type' => 'hidden',
          '#value' => $selected,
      ),
      'participant_id' => array(
          '#type' => 'hidden',
          '#value' => $node->nid,
      ),
      'submit' => array(
          '#type' => 'submit',
          '#value' => t('Submit'),
      )
  );
  return $form;
}

function adef_camps_registration_form_submit($form, &$form_state) {
  $participant_nid = $form_state['values']['participant_id'];
  $timespace = $form_state['values']['timespace'] != '' ? explode(',', $form_state['values']['timespace']) : array();
  $old_timespace = $form_state['values']['old_timespace'] != '' ? explode(',', $form_state['values']['old_timespace']) : array();
  $timespace_add = array_diff($timespace, $old_timespace);
  $timespace_remove = array_diff($old_timespace, $timespace);
  foreach ($timespace_add as $nid) {
    $node = node_load($nid);
    //check max participnats
    $session = node_load($node->field_session[LANGUAGE_NONE][0]['target_id']);
    $workshop = node_load($session->field_workshop[LANGUAGE_NONE][0]['target_id']);
    $max = $workshop->field_max_participants[LANGUAGE_NONE][0]['value'];
    $participants = 0;
    if (!empty($node->field_participants)) {
      $participants = count($node->field_participants[LANGUAGE_NONE]);
    }
    if ($participants < $max) {
      $node->field_participants[LANGUAGE_NONE][] = array(
          'target_id' => $participant_nid,
          'target_type' => 'node',
      );
      node_submit($node);
      node_save($node);
    } else {
      drupal_set_message(t('No more space in @session', array('@session' => $session->title)), 'warning');
    }
  }
  foreach ($timespace_remove as $nid) {
    $node = node_load($nid);
    $participants = $node->field_participants[LANGUAGE_NONE];
    foreach ($participants as $key => $item) {
      if ($item['target_id'] == $participant_nid) {
        unset($node->field_participants[LANGUAGE_NONE][$key]);
      }
    }
    if (count($node->field_participants[LANGUAGE_NONE]) == 0) {
      unset($node->field_participants[LANGUAGE_NONE]);
    }
    node_submit($node);
    node_save($node);
  }
  drupal_set_message(t('Registration is done.'));
}

function adef_camps_attendance($node) {
  $page['form'] = drupal_get_form('adef_camps_attendance_form', $node);
  return $page;
}

function adef_camps_attendance_form($form, &$form_state, $node) {
  //get registered attendance cases for the timespace
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
          ->entityCondition('bundle', 'attendance')
          ->fieldCondition('field_timespace', 'target_id', $node->nid, '=');
  $result = $query->execute();
  $nodes = array_keys($result['node']);

  $selected = array();
  foreach ($nodes as $nid) {
    $attendance_node = node_load($nid);
    $selected[] = $attendance_node->field_participants[LANGUAGE_NONE][0]['target_id'];
  }

  $form = array();

  //build participants options
  $form['participants'] = array(
      '#default_value' => $selected, //check registered cases
      '#required' => '0',
      '#key_type_toggled' => '0',
      '#type' => 'checkboxes',
      '#options' => array(),
      '#title' => t('Participants'),
  );
  $node_render = node_view($node);
  foreach ($node_render['field_participants']['#items'] as $participant) {
    $form['participants']['#options'][$participant['entity']->nid] = $participant['entity']->title;
  }
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
  );
  return $form;
}

function adef_camps_attendance_form_submit($form, &$form_state) {
  drupal_goto('node/6');
}

function adef_camps_registery_entry_form($form, &$form_state, $node, $type) {
  $form = array();
  $form['new'] = array(
      '#title' => t('Add new'),
      '#collapsed' => '1',
      '#type' => 'fieldset',
      '#collapsible' => '1',
  );
  $vid = _adef_camps_get_vid_byname('fields');
  $form['new']['track'] = array(
      '#required' => '0',
      '#multiple' => '0',
      '#key_type_toggled' => '0',
      '#type' => 'select',
      '#options' => _adef_camps_voc_to_array($vid),
      '#multiple_toggle' => '1',
      '#title' => t('In Track'),
  );
  $vid = _adef_camps_get_vid_byname('educational_goals');
  $form['new']['goal'] = array(
      '#required' => '0',
      '#multiple' => '0',
      '#key_type_toggled' => '0',
      '#type' => 'select',
      '#options' => _adef_camps_voc_to_array($vid),
      '#multiple_toggle' => '1',
      '#title' => t('Educational Goal'),
  );
  $form['new']['notes'] = array(
      '#required' => '1',
      '#type' => 'textarea',
      '#title' => t('Notes'),
  );
  $form['type'] = array(
      '#type' => 'hidden',
      '#value' => $type,
  );
  $form['participant_id'] = array(
      '#type' => 'hidden',
      '#value' => $node->nid,
  );
  $form['new']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
  );
  return $form;
}

function adef_camps_registery_entry_form_validate(&$form, &$form_state) {
  //drupal_set_message(print_r($form_state['values'], TRUE));
  if ($form_state['values']['track'] == '0' && $form_state['values']['goal'] == '0') {
    form_error($form['new']['goal'], t('Either "In Track" or "Educational Goal" needs to be set.'));
    form_error($form['new']['track']);
    $form['new']['#collapsed'] = '0';
  }
}

function adef_camps_registery_entry_form_submit($form, &$form_state) {
  global $user;
  $langcode = LANGUAGE_NONE;
  $node = new stdClass();
  $node->type = "register_entry";
  node_object_prepare($node); // Sets some defaults. Invokes hook_prepare() and hook_node_prepare().
  $node->language = $langcode; // Or e.g. 'en' if locale is enabled
  $node->uid = $user->uid;
  $node->status = 1; //(1 or 0): published or not
  $node->promote = 0; //(1 or 0): promoted to front page

  $node->body[LANGUAGE_NONE][0]['value'] = $form_state['values']['notes'];
  $node->field_register_type[LANGUAGE_NONE][]['value'] = $form_state['values']['type'];
  $node->field_register_participant[LANGUAGE_NONE][] = array(
      'target_id' => $form_state['values']['participant_id'],
      'target_type' => 'node',
  );
  if ($form_state['values']['goal'] != '0') {
    $node->field_register_edu_goal[LANGUAGE_NONE][]['tid'] = $form_state['values']['goal'];
  } else {
    $node->field_register_track[LANGUAGE_NONE][]['tid'] = $form_state['values']['track'];
  }
  $node = node_submit($node); // Prepare node for saving
  node_save($node);
  drupal_set_message(t('Registery entry added.'));
}

function adef_camps_badges($node) {
  $page['badges'] = array(
      '#theme' => 'item_list',
      '#items' => _adef_camps_badges_render($node->nid),
  );
  return $page;
}

function _adef_camps_badges_render($nid) {
  $items = array();
  $tracks = _adef_camps_voc_to_array(_adef_camps_get_vid_byname('fields'), FALSE);
  $goals = _adef_camps_voc_to_array(_adef_camps_get_vid_byname('educational_goals'), FALSE);
  $terms = $tracks + $goals;
  $system_badges = views_get_view_result('content_blocks', 'block_badges');
  $badges = _adef_camps_get_badges($nid);
  //drupal_set_message("<pre>" . print_r($badges, TRUE) . "</pre>");
  foreach ($badges as $key => $levels) {
    if (!empty($levels['attendance']) && count($levels['attendance']) >= 2) {
      foreach ($system_badges as $badge) {
        if ($badge->taxonomy_term_data_field_data_field_badge_term_tid == $key && intval($badge->field_field_badge_level[0]['raw']['value']) == 1) {
          $badge_name = $badge->node_title;
          break;
        }
      }
      if (isset($badge_name)) {
        $items[] = $badge_name;
      }
      if (!empty($levels['tracks']) && count($levels['tracks']) >= 2) {
        foreach ($system_badges as $badge) {
          if ($badge->taxonomy_term_data_field_data_field_badge_term_tid == $key && intval($badge->field_field_badge_level[0]['raw']['value']) == 2) {
            $badge_name = $badge->node_title;
            break;
          }
        }
        if (isset($badge_name)) {
          $items[] = $badge_name;
        }
        if (!empty($levels['tracks']) && count($levels['tracks']) >= 4) {
          foreach ($system_badges as $badge) {
            if ($badge->taxonomy_term_data_field_data_field_badge_term_tid == $key && intval($badge->field_field_badge_level[0]['raw']['value']) == 3) {
              $badge_name = $badge->node_title;
              break;
            }
          }
          if (isset($badge_name)) {
            $items[] = $badge_name;
          }
        }
      }
    }



    if (!empty($levels['edugoals'])) {
      if (count($levels['edugoals']) >= 2) {
        foreach ($system_badges as $badge) {
          if ($badge->taxonomy_term_data_field_data_field_badge_term_tid == $key && intval($badge->field_field_badge_level[0]['raw']['value']) == 1) {
            $badge_name = $badge->node_title;
            break;
          }
        }
        if (isset($badge_name)) {
          $items[] = $badge_name;
        }
        if (count($levels['edugoals']) >= 4) {
          foreach ($system_badges as $badge) {
            if ($badge->taxonomy_term_data_field_data_field_badge_term_tid == $key && intval($badge->field_field_badge_level[0]['raw']['value']) == 2) {
              $badge_name = $badge->node_title;
              break;
            }
          }
          if (isset($badge_name)) {
            $items[] = $badge_name;
          }
          if (count($levels['edugoals']) >= 6) {
            foreach ($system_badges as $badge) {
              if ($badge->taxonomy_term_data_field_data_field_badge_term_tid == $key && intval($badge->field_field_badge_level[0]['raw']['value']) == 3) {
                $badge_name = $badge->node_title;
                break;
              }
            }
            if (isset($badge_name)) {
              $items[] = $badge_name;
            }
          }
        }
      }
    }
  }
  return $items;
}

/*
 * Page to show all participants and their badges
 */

function adef_camps_badges_report() {
  $page = array();
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
          ->entityCondition('bundle', 'participant')
          ->propertyOrderBy('title');
  $result = $query->execute();
  $participant_nids = array_keys($result['node']);
  foreach ($participant_nids as $nid) {
    $node = node_load($nid, NULL, TRUE);
    $page[$nid] = array(
        '#type' => 'markup',
        '#markup' => '<h2>' . l($node->title, 'node/' . $node->nid) . '</h2>',
    );
    $page['badges-' . $nid] = array(
        '#theme' => 'item_list',
        '#items' => _adef_camps_badges_render($node->nid),
    );
  }
  return $page;
}

/*
 * Returns array of badges for participant with $nid
 */

function _adef_camps_get_badges($nid) {
  $badges = array();
  $attendance = views_get_view_result('content_blocks', 'block_participant_attendance', $nid);
  foreach ($attendance as $session) {
    $key = $session->field_field_tech_field[0]['raw']['tid'];
    $value = $session->nid;
    if (empty($badges[$key]['attendance'])) {
      $badges[$key]['attendance'] = array();
    }
    $badges[$session->field_field_tech_field[0]['raw']['tid']]['attendance'][] = $session->nid;
  }
  $achievments = views_get_view_result('content_blocks', 'block_participant_registry', $nid, 'achievement');
  //drupal_set_message("<pre>" . print_r($achievments, TRUE) . "</pre>");
  foreach ($achievments as $register) {
    if (!empty($register->field_field_register_edu_goal)) {
      $key = $register->field_field_register_edu_goal[0]['raw']['tid'];
      $value = $register->nid;
      if (empty($badges[$key]['edugoals'])) {
        $badges[$key]['edugoals'] = array();
      }
      $badges[$register->field_field_register_edu_goal[0]['raw']['tid']]['edugoals'][] = $register->nid;
    }
    if (!empty($register->field_field_register_track)) {
      $key = $register->field_field_register_track[0]['raw']['tid'];
      $value = $register->nid;
      if (empty($badges[$key]['tracks'])) {
        $badges[$key]['tracks'] = array();
      }
      $badges[$register->field_field_register_track[0]['raw']['tid']]['tracks'][] = $register->nid;
    }
  }
//  foreach ($session->field_field_edu_goals as $edugoal) {
//    $key = $edugoal['raw']['tid'];
//    $value = $session->nid;
//    if (empty($badges[$key]['attendance'])) {
//      $badges[$key]['attendance'] = array();
//    }
//    $badges[$key]['attendance'][] = $value;
//  }
  return $badges;
}

function _adef_camps_voc_to_array($vid, $add_blank = TRUE) {
  $terms = taxonomy_get_tree($vid);
  if ($add_blank) {
    $output[0] = '';
  }
  foreach ($terms as $data) {
    $output[$data->tid] = $data->name;
  }
  return $output;
}

function _adef_camps_get_vid_byname($vocabulary) {
  $v = taxonomy_vocabulary_machine_name_load($vocabulary);
  return $v->vid;
}