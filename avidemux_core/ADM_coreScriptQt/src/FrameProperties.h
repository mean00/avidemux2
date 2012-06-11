#ifndef ADM_qtScript_FrameProperties
#define ADM_qtScript_FrameProperties

#include "ADM_inttype.h"
#include "QtScriptObject.h"

namespace ADM_qtScript
{
	/** \brief The FrameProperties %class holds the properties of a video frame.
	 */
	class FrameProperties : public QtScriptObject
	{
		Q_OBJECT
		Q_ENUMS(CodingType FieldType)

	public:

		/** \brief Specifies the coding type of a video frame.
		 */
		enum CodingType
		{
			UnknownFrame = 0, /**< Unknown coded picture */
			I_Frame = 1, /**< Intra-coded picture */
			P_Frame = 2, /**< Predicted picture */
			B_Frame = 3 /**< Bi-predictive picture */
		};

		/** \brief Specifies the field type of a video frame.
		 */
		enum FieldType
		{
			UnknownField = 0, /**< Unknown field type */
			TopField = 1,     /**< Top field of frame */
			BottomField = 2,  /**< Bottom field of frame */
			NoField = 3       /**< Complete frame */
		};

	private:
		uint64_t _time;
		FieldType _fieldType;
		CodingType _frameType;
		uint32_t _quantiser;

		QScriptValue getQuantiser(void);
		QScriptValue getTime(void);
		QScriptValue getFieldType(void);
		QScriptValue getFrameType(void);
		CodingType getFrameType(int frameType);
		FieldType getFieldType(int frameType);

	public:
		/** \cond */
		FrameProperties(IEditor *editor, uint64_t time);
		/** \endcond */

		/** \brief Returns the field type of the frame.
		 */
		Q_PROPERTY(QScriptValue /*% FieldType %*/ fieldType READ getFieldType);

		/** \brief Returns the coding type of the frame.
		 */
		Q_PROPERTY(QScriptValue /*% CodingType %*/ frameType READ getFrameType);

		/** \brief Returns the average quantiser of the frame.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ quantiser READ getQuantiser);

		/** \brief Returns the time (in milliseconds) of the frame.
		 *
		 * \sa Editor.currentFrameTime
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ time READ getTime);
	};
}
#endif
