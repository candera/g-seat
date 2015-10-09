(ns meter.core
  (:require [fx-clj.core :as fx]
            [clojure.core.async :as async]))

(def button-ch (async/chan))

(defn create-view []
  (fx/h-box
   (fx/v-box
    (fx/slider :#a0
               {:max 1.0
                :min 0.0
                :value 0.5
                :disable true
                :orientation :vertical
                :major-tick-unit 0.25
                :minor-tick-count 4
                :show-tick-labels true})
    (fx/label "A0"))
   (fx/v-box
    (fx/slider :#a1
               {:max 1.0
                :min 0.0
                :value 0.5
                :orientation :vertical
                :major-tick-unit 0.25
                :minor-tick-count 4
                :show-tick-labels true})
    (fx/label "A1"))))

(defn go []
  (fx/sandbox #'create-view))

(defn set-slider [stage slider val]
  (fx/run!
   (fx/pset! (fx/lookup (.getScene stage) slider) {:value val})))

