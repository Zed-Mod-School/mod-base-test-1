;; "project file" for text make tool.
;; it's very simple... a list of (action args...)
;; There are two actions:
;; - file-json (A path to a json file)
;;   - args: (language_id text_version group_name '(filename list))
;; - file (A path to a GOAL data file)
;;   - the same arguments are provided within the file itself

(text
  ;; NOTE : we compile using the fixed v2 encoding because it's what we use.
  (file "$DECOMP/assets/game_text.txt") ;; this is the decompiler-generated file!
  ;; add custom files down here
  ;; built-in languages
  (file-json 0 jak2 "common" '("game/assets/jak2/text/game_custom_text_en-US.json"))
  (file-json 1 jak2 "common" '("game/assets/jak2/text/game_custom_text_fr-FR.json"))
  (file-json 2 jak2 "common" '("game/assets/jak2/text/game_custom_text_de-DE.json"))
  (file-json 3 jak2 "common" '("game/assets/jak2/text/game_custom_text_es-ES.json"))
  (file-json 4 jak2 "common" '("game/assets/jak2/text/game_custom_text_it-IT.json"))
  (file-json 5 jak2 "common" '("game/assets/jak2/text/game_custom_text_ja-JP.json"))
  (file-json 6 jak2 "common" '("game/assets/jak2/text/game_custom_text_ko-KO.json"))
  (file-json 7 jak2 "common" '("game/assets/jak2/text/game_custom_text_en-GB.json"))
  ;; custom languages
  (file-json 8 jak2 "common" '("game/assets/jak2/text/game_custom_text_pt-PT.json"))
  (file-json 9 jak2 "common" '("game/assets/jak2/text/game_custom_text_fi-FI.json"
                               "game/assets/jak2/text/game_base_text_fi-FI.json"))
  (file-json 10 jak2 "common" '("game/assets/jak2/text/game_custom_text_sv-SE.json"))
  (file-json 11 jak2 "common" '("game/assets/jak2/text/game_custom_text_da-DK.json"))
  (file-json 12 jak2 "common" '("game/assets/jak2/text/game_custom_text_no-NO.json"))
  (file-json 13 jak2 "common" '("game/assets/jak2/text/game_custom_text_nl-NL.json"))
  (file-json 14 jak2 "common" '("game/assets/jak2/text/game_custom_text_pt-BR.json"))
  (file-json 15 jak2 "common" '("game/assets/jak2/text/game_custom_text_hu-HU.json"))
  (file-json 16 jak2 "common" '("game/assets/jak2/text/game_custom_text_ca-ES.json"))
  (file-json 17 jak2 "common" '("game/assets/jak2/text/game_custom_text_is-IS.json"))
  (file-json 18 jak2 "common" '("game/assets/jak2/text/game_custom_text_pl-PL.json"))
  (file-json 19 jak2 "common" '("game/assets/jak2/text/game_custom_text_lt-LT.json"))
  (file-json 20 jak2 "common" '("game/assets/jak2/text/game_custom_text_cs-CZ.json"))
  (file-json 21 jak2 "common" '("game/assets/jak2/text/game_custom_text_hr-HR.json"))
  (file-json 22 jak2 "common" '("game/assets/jak2/text/game_custom_text_gl-ES.json"))
  )
