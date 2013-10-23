<?php

function adef_camps_registration_slots() {
  $result = array();
  $period_date = adef_camps_get_period_date();
  $views_date_arg = $period_date['year'] . '-' . $period_date['month'] . '-' . $period_date['day'] . '--P2D';
  $sessions = views_get_view_result('content_blocks', 'block_sessions', $views_date_arg);
  foreach ($sessions as $timespace) {
    $node = node_load($timespace->nid);
    if (!empty($node->field_session)) {
      $session = node_load($node->field_session[LANGUAGE_NONE][0]['target_id']);
      $workshop = node_load($session->field_workshop[LANGUAGE_NONE][0]['target_id']);
      $max = intval($workshop->field_max_participants[LANGUAGE_NONE][0]['value']);
      $participants = 0;
      if (!empty($node->field_participants)) {
        $participants = count($node->field_participants[LANGUAGE_NONE]);
      }
      $remain = $max-$participants;
      $result[$timespace->nid] = $remain;
    }
  }
  drupal_json_output($result);
}