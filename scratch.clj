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
