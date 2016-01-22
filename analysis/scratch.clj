(doto (charts/scatter-plot [] [] :legend true)
  (charts/add-points [1 2 3] [4 5 6] :series-label "Blah")
  (charts/add-points [3 2 1] [4 5 6] :series-label "Blerg")
  (incanter/view))

(let [data(->> (for [drive [-2500 2500]]
                 (->>
                  (run-test "/dev/cu.usbmodem1451" drive drive)
                       (map #(assoc % :series 1))))
                (reduce into))]
  (incanter/with-data (incanter/to-dataset data)
    (doto (charts/scatter-plot :x :dxdt)
      (incanter/view))))

(let [chart (charts/scatter-plot nil nil :legend true)]
  (doseq [drive [-10000 -5000 -2500 -1500 1500 2500 5000 10000]]
    (incanter/with-data
      (incanter/to-dataset
       (run-test "/dev/cu.usbmodem1451" drive))
      (charts/add-points chart :pos :dxdt :series-label drive)))
  (incanter/view chart))

(doto (charts/scatter-plot)
  (charts/add-lines [1 2 3] [4 5 6])
  (charts/add-lines [6 7 8] [9 10 11])
  (incanter/view))


(let [data (move "/dev/cu.usbmodem1451" 5000)
      chart (fn [data x y title]
              (incanter/view (charts/scatter-plot
                              (map x data)
                              (map y data)
                              :title title)))]
  (chart data :t :pos "pos")
  (chart data :t :drive "drive")
  (chart data :t :a "a")
  (chart data :t :v "v")
  (chart data :t :target-v "Target V")
  (chart data :t #(- (:v %) (:target-v %))
         "V - Target V"))

(let [data (move "/dev/cu.usbmodem1451" 4500)]
  (two-axis-chart :t "t" :drive "drive" :v "v" data)
  (two-axis-chart :t "t" :target-v "target-v" :v "v" data)
  (two-axis-chart :t "t" :pos "pos" :drive "drive" data)
  (two-axis-chart :t "t" :pos "pos" :v "v" data))

(let [data (run-data-command "/dev/cu.usbmodem1451" "TEST BL 7000\n")]
  (two-axis-chart :t "t" :drive "drive" :v "v" data)
  ;; (two-axis-chart :t "t" :target-v "target-v" :v "v" data)
  (two-axis-chart :t "t" :pos "pos" :drive "drive" data)
  (two-axis-chart :t "t" :pos "pos" :v "v" data))

(let [data (->> (run-data-command "/dev/cu.usbmodem1411" "M BL 2000")
                (filter #(= "BL" (:ch %))))]
  (two-axis-chart :t "t" :drive "drive" :v "v" data)
  (two-axis-chart :t "t" :target-v "target-v" :v "v" data)
  (two-axis-chart :t "t" :pos "pos" :drive "drive" data)
  (two-axis-chart :t "t" :pos "pos" :v "v" data))


(run-data-command "/dev/cu.usbmodem1411" "Q")

(->> (analysis.core/dump-command "/dev/cu.usbmodem1411" "Q" 2)
     (map analysis.core/parse-line))

(dotimes [i 60]
  (->> (analysis.core/dump-command "/dev/cu.usbmodem1411" "Q" 4)
       (map analysis.core/parse-line)
       (map (fn [{:keys [pos]}] pos))
       println)
  (Thread/sleep 1000))

(move "/dev/cu.usbmodem1411" 3000)


(let [mapify (fn [item]
               (->> item
                    (map #(clojure.string/split % #"="))
                    (filter #(= (count %) 2))
                    (into {})))]
  (->> "/tmp/tel.txt"
       clojure.java.io/file
       clojure.java.io/reader
       line-seq
       (map #(clojure.string/split % #","))
       (map mapify)
       (remove empty?)
       ))
