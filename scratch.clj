(require '[clojure.java.io :as io])

(def p (io/writer "/dev/cu.usbmodem14121"))

(spit p "GOTO 950\n")

(let [min 670
      max 1020]
 (defn goto [n]
   (let [target (-> (- max min) (* n) (+ min) long)]
     (spit "/dev/cu.usbmodem14121" (format "GOTO %d\n" target)))))

(defn smooth []
  (doseq [n (range 800 1020 2)]
    (goto n)
    (Thread/sleep 1)))

(dotimes [_ 10]
  (doseq [n (range 0 1 0.04)]
    (goto n)
    (Thread/sleep 1))
  (doseq [n (range 1 0 -0.01)]
    (goto n)
    (Thread/sleep 1)))

(defn sin [rad] (Math/sin rad))
(defn cos [rad] (Math/cos rad))

(defn mypr [yaw pitch roll]
  (let [[sy sp sr] (map sin [yaw pitch roll])
        [cy cp cr] (map cos [yaw pitch roll])]
    [[(* cp cy)
      (- (* cy sp sr) (* sy cr))
      (+ (* cy cr sp) (* sy sr))]
     [(* sy cp)
      (+ (* sy sp sr) (* cr cy))
      (- (* sy cr sp) (* cy sr))]
     [(- sp)
      (* sr cp)
      (* cr cp)]]))

(defn deg [deg]
  (/ (* deg Math/PI) 180.0))

(defn xform [[i j k :as v] [yaw pitch roll]]
  (let [m (mypr yaw pitch roll)]
    (mapv #(reduce + (map * v %)) m)))

(defn scale-to
  [val in1 in2 out1 out2]
  (+ out1
     (* (/ (- val in1) (- in2 in1))
        (- out2 out1))))

(defn nonlinear
  [x]
  (- 1.0 (/ 1.0 (+ 1.0 x))))

(defn position [g base-g]
  (if (< base-g g)
    (scale-to (nonlinear (- g base-g))
              0 1 0.25 1.0)
    (scale-to (nonlinear (- base-g g))
              0 1 0.25 0)))

(let [t0 (.getTime (java.util.Date.))]
  (defn now
    []
    (/ (- (.getTime (java.util.Date.))
          t0)
       1000.0)))

(defn replay [device path]
  (let [t0 (now)]
    (with-open [output (clojure.java.io/writer device)
                in (clojure.java.io/reader path)]
      (loop [tstart nil
             [line & more] (line-seq in)]
        (when line
          (let [[t xdot ydot zdot yaw pitch roll bl br sl sr]
                (map #(Float. %) (clojure.string/split line #", "))]
            (when tstart
              (let [real-elapsed (- (now) t0)
                    playback-elapsed (- t tstart)]
                (when (< real-elapsed playback-elapsed)
                  (Thread/sleep (long (* 1000 (- playback-elapsed real-elapsed)))))))
            (doseq [[ch val] [["BL" bl]
                              ["BR" br]
                              ;; ["SL" sl]
                              ;; ["SR" sr]
                              ]]
              (let [cmd (format "M %s %d\n"
                                ch
                                (-> val (* 10000) long))]
                (.write output cmd)
                (println "T" t)
                (print cmd)
                ))
            (println "----------------\n")
            (.flush output)
            (.flush *out*)
            (recur (or tstart t) more)))))))

(spit "oscillation.csv"
      (with-out-str
        (doseq [t (range 0.0 60 0.01)]
          (let [pos (-> t (* 0.75) Math/sin (+ 1.0) (/ 2.0))]
           (println (format "%f, 0, 0, 0, 0, 0, 0, %f, %f, %f, %f"
                            t
                            pos
                            pos
                            pos
                            pos))))))

(spit "square.csv"
      (with-out-str
        (doseq [t (range 0 10)]
          (let [pos (if (even? t) 0.0 1.0)]
            (println (format "%f, 0, 0, 0, 0, 0, 0, %f, %f, %f, %f"
                             (float t)
                             pos
                             pos
                             pos
                             pos))))))

(replay "/dev/cu.usbmodem1411" "../oscillation.csv")
(replay "/dev/cu.usbmodem1411" "square.csv")
(replay "/dev/cu.usbmodem1411" "../flight-data-1.csv")


(defn run-test [device drive]
  (with-open [dev-out (clojure.java.io/writer device)
              dev-in (clojure.java.io/reader device)]
    (.write dev-out (format "TEST BL %d\n" drive))
    (.flush dev-out)
    (let [[header & lines] (line-seq dev-in)]
      (->> (loop [data []
                  [line & more] lines]
             (println "'" line "'")
             (.flush *out*)
             (if (clojure.string/blank? line)
               data
               (let [[t x] (map #(Long. %) (clojure.string/split line #","))]
                 (recur (conj data {:t t :x x :drive drive}) more))))))))

(defn run-tests [device drives out]
  (->> drives
       (mapcat #(run-test device %))
       doall
       (map-indexed (fn [row {:keys [drive t x]}]
                      (format "%d,%d,%d,=B%d-B%d,=C%d-C%d,=E%d/D%d"
                              drive t x
                              row (inc row)
                              row (inc row)
                              (inc row) (inc row))))
       (interpose "\n")
       (apply str)
       (spit out)))

(doseq [drive ;;(range -10000 10001 1000)
        [5000 -5000]]
  (spit (format "/tmp/test-%d.csv" drive)
        (run-test "/dev/cu.usbmodem1451" drive)))
