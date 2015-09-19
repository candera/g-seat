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
  (doseq [n (range 0 1 0.01)]
    (goto n)
    (Thread/sleep 2))
  (doseq [n (range 1 0 -0.01)]
    (goto n)
    (Thread/sleep 2)))


